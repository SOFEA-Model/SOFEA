// Modified from qtcurl source:
// https://github.com/tarasvb/qtcurl/tree/e0987f2
//

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "CurlMulti.h"
#include "CurlEasy.h"

#include <QSocketNotifier>
#include <QTimer>

#include <limits>
#include <memory>

struct CurlMultiSocket
{
    curl_socket_t socketDescriptor = CURL_SOCKET_BAD;
    QSocketNotifier *readNotifier = nullptr;
    QSocketNotifier *writeNotifier = nullptr;
    QSocketNotifier *errorNotifier = nullptr;
};

CurlMulti::CurlMulti(QObject *parent)
    : QObject(parent)
    , timer_(new QTimer(this))
{
    handle_ = curl_multi_init();
    Q_ASSERT(handle_ != nullptr);

    curl_multi_setopt(handle_, CURLMOPT_SOCKETFUNCTION, staticCurlSocketFunction);
    curl_multi_setopt(handle_, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(handle_, CURLMOPT_TIMERFUNCTION, staticCurlTimerFunction);
    curl_multi_setopt(handle_, CURLMOPT_TIMERDATA, this);

    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &CurlMulti::curlMultiTimeout);
}

CurlMulti::~CurlMulti()
{
    while (!transfers_.empty()) {
        (*transfers_.begin())->abort();
    }

    if (handle_) {
        curl_multi_cleanup(handle_);
    }
}

CurlMulti *CurlMulti::threadInstance()
{
    thread_local auto instances = std::make_shared<CurlMulti>();
    return instances.get();
}

void CurlMulti::addTransfer(CurlEasy *transfer)
{
    transfers_.insert(transfer);
    curl_multi_add_handle(handle_, transfer->handle());
}

void CurlMulti::removeTransfer(CurlEasy *transfer)
{
    auto it = transfers_.find(transfer);
    if (it != transfers_.end()) {
        curl_multi_remove_handle(handle_, transfer->handle());
        transfers_.erase(transfer);
    }
}

int CurlMulti::curlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int action, CurlMultiSocket *socket)
{
    Q_UNUSED(easyHandle);

    if (!socket) {
        if (action == CURL_POLL_REMOVE || action == CURL_POLL_NONE)
            return 0;

        socket = new CurlMultiSocket;
        socket->socketDescriptor = socketDescriptor;
        curl_multi_assign(handle_, socketDescriptor, socket);
    }

    if (action == CURL_POLL_REMOVE) {
        curl_multi_assign(handle_, socketDescriptor, nullptr);

        // Note: deleteLater will NOT work here since there are
        //       situations where curl subscribes same socket descriptor
        //       until events processing is done and actual delete happen.
        //       This causes QSocketNotifier not to register notifications again.
        if (socket->readNotifier) delete socket->readNotifier;
        if (socket->writeNotifier) delete socket->writeNotifier;
        if (socket->errorNotifier) delete socket->errorNotifier;
        delete socket;
        return 0;
    }

    if (action == CURL_POLL_IN || action == CURL_POLL_INOUT) {
        if (!socket->readNotifier) {
            socket->readNotifier = new QSocketNotifier(socket->socketDescriptor, QSocketNotifier::Read);
            connect(socket->readNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyRead);
        }
        socket->readNotifier->setEnabled(true);
    } else {
        if (socket->readNotifier)
            socket->readNotifier->setEnabled(false);
    }

    if (action == CURL_POLL_OUT || action == CURL_POLL_INOUT) {
        if (!socket->writeNotifier) {
            socket->writeNotifier = new QSocketNotifier(socket->socketDescriptor, QSocketNotifier::Write);
            connect(socket->writeNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyWrite);
        }
        socket->writeNotifier->setEnabled(true);
    } else {
        if (socket->writeNotifier)
            socket->writeNotifier->setEnabled(false);
    }

    return 0;
}

int CurlMulti::curlTimerFunction(int timeoutMsec)
{
    if (timeoutMsec >= 0)
        timer_->start(timeoutMsec);
    else
        timer_->stop();

    return 0;
}

void CurlMulti::curlMultiTimeout()
    { curlSocketAction(CURL_SOCKET_TIMEOUT, 0); }

void CurlMulti::socketReadyRead(int socketDescriptor)
    { curlSocketAction(socketDescriptor, CURL_CSELECT_IN); }

void CurlMulti::socketReadyWrite(int socketDescriptor)
    { curlSocketAction(socketDescriptor, CURL_CSELECT_OUT); }

void CurlMulti::socketException(int socketDescriptor)
    { curlSocketAction(socketDescriptor, CURL_CSELECT_ERR); }

void CurlMulti::curlSocketAction(curl_socket_t socketDescriptor, int eventsBitmask)
{
    int runningHandles;
    CURLMcode rc = curl_multi_socket_action(handle_, socketDescriptor, eventsBitmask, &runningHandles);
    if (rc != 0) {
        // TODO: Handle global curl errors
    }

    int messagesLeft = 0;
    do {
        CURLMsg *message = curl_multi_info_read(handle_, &messagesLeft);

        if (message == nullptr)
            break;

        if (message->easy_handle == nullptr)
            continue;

        CurlEasy *transfer = nullptr;
        curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &transfer);

        if (transfer == nullptr)
            continue;

        transfer->onCurlMessage(message);
    } while (messagesLeft);
}

int CurlMulti::staticCurlSocketFunction(CURL *easyHandle, curl_socket_t socketDescriptor, int what, void *userp, void *sockp)
{
    Q_UNUSED(easyHandle);
    CurlMulti *multi = static_cast<CurlMulti*>(userp);
    Q_ASSERT(multi != nullptr);

    return multi->curlSocketFunction(easyHandle, socketDescriptor, what, static_cast<CurlMultiSocket*>(sockp));
}

int CurlMulti::staticCurlTimerFunction(CURLM *multiHandle, long timeoutMs, void *userp)
{
    Q_UNUSED(multiHandle);
    CurlMulti *multi = static_cast<CurlMulti*>(userp);
    Q_ASSERT(multi != nullptr);

    int intTimeoutMs;

    if (timeoutMs >= std::numeric_limits<int>::max())
        intTimeoutMs = std::numeric_limits<int>::max();
    else if (timeoutMs >= 0)
        intTimeoutMs = static_cast<int>(timeoutMs);
    else
        intTimeoutMs = -1;

    return multi->curlTimerFunction(intTimeoutMs);
}

