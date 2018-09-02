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
