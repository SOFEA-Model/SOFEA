#include <QDataStream>

#include "IPCServer.h"

const QString PIPENAME = "\\\\.\\pipe\\AERMODProgressPipe";

IPCServer::IPCServer(QObject *parent) : QObject(parent)
{
    server = new QLocalServer(this);
}

bool IPCServer::start()
{
    if (!server->listen(PIPENAME)) {
        qDebug() << QString("Unable to start the IPC server: %1").arg(server->errorString());
        return false;
    }
    qDebug() << "IPC server started.";
    connect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    return true;
}

bool IPCServer::stop()
{
    server->close();
    if (server->isListening()) {
        qDebug() << QString("Unable to stop the IPC server: %1").arg(server->errorString());
        return false;
    }
    qDebug() << "IPC server stopped.";
    disconnect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    connections.clear();
    return true;
}

void IPCServer::clientConnected()
{
    qDebug() << "IPC client connected.";

    QLocalSocket *socket = server->nextPendingConnection();
    connections.push_back(socket);
    connect(socket, &QLocalSocket::disconnected, this, &IPCServer::clientDisconnected);
    connect(socket, &QLocalSocket::readyRead, this, &IPCServer::readMessage);
}

void IPCServer::clientDisconnected()
{
    qDebug() << "IPC client disconnected.";

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
