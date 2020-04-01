// Modified from qtcurl source:
// https://github.com/tarasvb/qtcurl/tree/e0987f2
//

#pragma once

#include <QObject>

#include <set>

#include <curl/curl.h>

class CurlEasy;
struct CurlMultiSocket;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class CurlMulti : public QObject
{
    Q_OBJECT

public:
    explicit CurlMulti(QObject *parent = nullptr);
    virtual ~CurlMulti();

    static CurlMulti* threadInstance();

    void addTransfer(CurlEasy *transfer);
    void removeTransfer(CurlEasy *transfer);

protected slots:
    void curlMultiTimeout();
    void socketReadyRead(int socketDescriptor);
    void socketReadyWrite(int socketDescriptor);
    void socketException(int socketDescriptor);

protected:
    void curlSocketAction(curl_socket_t socketDescriptor, int eventsBitmask);
    int curlTimerFunction(int timeoutMsec);
    int curlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int action, CurlMultiSocket *socket);
    static int staticCurlTimerFunction(CURLM *multiHandle, long timeoutMs, void *userp);
    static int staticCurlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int what, void *userp, void *sockp);

    QTimer *timer_ = nullptr;
    CURLM *handle_ = nullptr;

    std::set<CurlEasy *> transfers_;
};

