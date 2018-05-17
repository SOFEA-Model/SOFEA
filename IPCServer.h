#ifndef IPCSERVER_H
#define IPCSERVER_H

#include <QLocalServer>
#include <QLocalSocket>

class IPCServer : public QObject
{
    Q_OBJECT

public:
    explicit IPCServer(QObject *parent = nullptr);
    bool start();
    bool stop();

private slots:
    void clientConnected();
    void clientDisconnected();
    void readMessage();

signals:
    void messageReceived(quint32 pid, quint32 msg);

private:
    QLocalServer *server;
    QList<QLocalSocket *> connections;
};

#endif // IPCSERVER_H
