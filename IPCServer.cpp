#include <QDataStream>

#include "IPCServer.h"

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

const QString PIPENAME = "\\\\.\\pipe\\AERMODProgressPipe";

IPCServer::IPCServer(QObject *parent) : QObject(parent)
{
    server = new QLocalServer(this);
}

bool IPCServer::start()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Model");

    if (!server->listen(PIPENAME)) {
        BOOST_LOG_TRIVIAL(error) << "Unable to start the IPC server: "
                                 << server->errorString().toStdString();
        return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "IPC server started";
    connect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    return true;
}

bool IPCServer::stop()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Model");

    server->close();
    if (server->isListening()) {
        BOOST_LOG_TRIVIAL(error) << "Unable to stop the IPC server: "
                                 << server->errorString().toStdString();
        return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "IPC server stopped";
    disconnect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    connections.clear();
    return true;
}

void IPCServer::clientConnected()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Model");
    BOOST_LOG_TRIVIAL(debug) << "IPC client connected";

    QLocalSocket *socket = server->nextPendingConnection();
    connections.push_back(socket);
    connect(socket, &QLocalSocket::disconnected, this, &IPCServer::clientDisconnected);
    connect(socket, &QLocalSocket::readyRead, this, &IPCServer::readMessage);
}

void IPCServer::clientDisconnected()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Model");
    BOOST_LOG_TRIVIAL(debug) << "IPC client disconnected";

    QLocalSocket *socket = qobject_cast<QLocalSocket*>(QObject::sender());
    connections.removeAll(socket);
    socket->deleteLater();
}

static inline quint32 fromNetworkData(const char *data)
{
    const unsigned char *udata = (const unsigned char *)data;

    // byte array is little endian.
    return (quint32(udata[3]) << 24)
         | (quint32(udata[2]) << 16)
         | (quint32(udata[1]) << 8)
         | (quint32(udata[0]));
}

void IPCServer::readMessage()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(QObject::sender());

    if (socket->bytesAvailable() < 8)
        socket->waitForReadyRead(100);

    char message[8];
    socket->read(message, 8);

    char pidbuf[4];
    char msgbuf[4];
    memcpy(&pidbuf, message,   4);
    memcpy(&msgbuf, message+4, 4);

    quint32 pid = fromNetworkData(pidbuf);
    quint32 msg = fromNetworkData(msgbuf);

    emit messageReceived(pid, msg);
}
