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

#ifndef IPCSERVER_H
#define IPCSERVER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QMap>

#include <string>

class IPCServer : public QObject
{
    Q_OBJECT

public:
    explicit IPCServer(QObject *parent = nullptr);
    bool start();
    bool stop();
    void addPid(const qint64 pid, const std::string &dir);
    void removePid(const qint64 pid);

private slots:
    void clientConnected();
    void clientDisconnected();
    void readMessage();

signals:
    void messageReceived(quint32 pid, quint32 msg);

private:
    QLocalServer *server;
    QList<QLocalSocket *> connections;
    QMap<qint64, std::string> pidToDir;
};

#endif // IPCSERVER_H
