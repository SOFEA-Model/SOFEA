// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "IPCServer.h"
#include "IPCMessage.h"

#include <QByteArray>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

const QString PIPENAME = "\\\\.\\pipe\\AERMODStatusPipe";

IPCServer::IPCServer(QObject *parent) : QObject(parent)
{
    server = new QLocalServer(this);
}

bool IPCServer::start()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");

    // QLocalServer will open a pipe for asynchronous I/O (FILE_FLAG_OVERLAPPED).
    // See Qt sources: src/network/socket/qlocalserver_win.cpp

    if (!server->listen(PIPENAME)) {
        BOOST_LOG_TRIVIAL(error) << "Unable to start the IPC server: "
                                 << server->errorString().toStdString();
        return false;
    }

    //BOOST_LOG_TRIVIAL(debug) << "IPC server started";
    connect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    return true;
}

bool IPCServer::stop()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");

    server->close();
    if (server->isListening()) {
        BOOST_LOG_TRIVIAL(error) << "Unable to stop the IPC server: "
                                 << server->errorString().toStdString();
        return false;
    }

    //BOOST_LOG_TRIVIAL(debug) << "IPC server stopped";
    disconnect(server, &QLocalServer::newConnection, this, &IPCServer::clientConnected);
    connections.clear();
    return true;
}

void IPCServer::addPid(const qint64 pid, const std::string& dir)
{
    pidToDir[pid] = dir;
}

void IPCServer::removePid(const qint64 pid)
{
    pidToDir.remove(pid);
}

void IPCServer::clientConnected()
{
    //BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");
    //BOOST_LOG_TRIVIAL(debug) << "IPC client connected";

    QLocalSocket *socket = server->nextPendingConnection();
    connections.push_back(socket);
    connect(socket, &QLocalSocket::disconnected, this, &IPCServer::clientDisconnected);
    connect(socket, &QLocalSocket::readyRead, this, &IPCServer::readMessage);
}

void IPCServer::clientDisconnected()
{
    //BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");
    //BOOST_LOG_TRIVIAL(debug) << "IPC client disconnected";

    QLocalSocket *socket = qobject_cast<QLocalSocket*>(QObject::sender());
    connections.removeAll(socket);
    socket->deleteLater();
}

void IPCServer::readMessage()
{
    using namespace IPCMessage;

    QLocalSocket *socket = qobject_cast<QLocalSocket*>(QObject::sender());
    QByteArray buffer = socket->readAll();

    int offset = 0;
    while (true)
    {
        // Read message header.
        if (offset + sizeof(header_t) > buffer.size())
            break;

        IPCMessage::header_t header;
        std::memcpy(&header, buffer.constData() + offset, sizeof(header));

        // Read full message.
        if (offset + header.cbsize > buffer.size())
            break;

        if (header.msgtyp == 1 && header.cbsize == sizeof(errmsg_buffer_t))
        {
            IPCMessage::errmsg_buffer_t msgbuf;
            std::memcpy(&msgbuf, buffer.constData() + offset, sizeof(msgbuf));

            // Fortran char arrays are not null-terminated.
            std::string pathwy(msgbuf.pathwy, sizeof(msgbuf.pathwy));
            std::string errcod(msgbuf.errcod, sizeof(msgbuf.errcod));
            int lineno = msgbuf.lineno;
            std::string modnam(msgbuf.modnam, sizeof(msgbuf.modnam));
            std::string errmg1(msgbuf.errmg1, sizeof(msgbuf.errmg1));
            std::string errmg2(msgbuf.errmg2, sizeof(msgbuf.errmg2));
            boost::trim_right(errmg1);
            boost::trim_right(errmg2);

            // Set attributes.
            std::string dir = pidToDir.value(header.procid);
            BOOST_LOG_SCOPED_THREAD_TAG("Dir", dir);
            BOOST_LOG_SCOPED_THREAD_TAG("Pathway", pathwy);
            BOOST_LOG_SCOPED_THREAD_TAG("ErrorCode", errcod);
            BOOST_LOG_SCOPED_THREAD_TAG("Line", lineno);
            BOOST_LOG_SCOPED_THREAD_TAG("Module", modnam);

            // Log the error message.
            switch (msgbuf.errtyp) {
            case 'E':
                BOOST_LOG_TRIVIAL(error) << errmg1 << " " << errmg2;
                break;
            case 'W':
                BOOST_LOG_TRIVIAL(warning) << errmg1 << " " << errmg2;
                break;
            default:
                BOOST_LOG_TRIVIAL(info) << errmg1 << " " << errmg2;
                break;
            }
        }
        else if (header.msgtyp == 2 && header.cbsize == sizeof(tothrs_buffer_t))
        {
            IPCMessage::tothrs_buffer_t msgbuf;
            std::memcpy(&msgbuf, buffer.constData() + offset, sizeof(msgbuf));

            // Signal the Run Model dialog to update progress indicators.
            emit messageReceived(msgbuf.header.procid, msgbuf.tothrs);
        }

        offset += header.cbsize;
    }
}
