// Modified from qtcurl source:
// https://github.com/tarasvb/qtcurl/tree/e0987f2
//

#pragma once

#include <QObject>
#include <QUrl>

#include <functional>
#include <map>

#include <curl/curl.h>

class CurlMulti;

class CurlEasy : public QObject
{
    Q_OBJECT

public:
    using DataFunction = std::function<size_t(char *buffer, size_t size)>;
    using SeekFunction = std::function<int(qint64 offset, int origin)>;

    explicit CurlEasy(QObject *parent = nullptr);
    virtual ~CurlEasy();

    void perform();
    void abort();
    bool isRunning() { return runningOnMulti_ != nullptr; }
    CURLcode result() { return lastResult_; }

    // For the list of available set options and valid parameter types consult curl_easy_setopt manual
    template <typename T> bool set(CURLoption option, T parameter) { return curl_easy_setopt(handle_, option, parameter) == CURLE_OK; }
    bool set(CURLoption option, const QString &parameter); // Convenience override for const char* parameters
    bool set(CURLoption option, const QUrl &parameter); // Convenience override for const char* parameters
    void setReadFunction(const DataFunction &function);
    void setWriteFunction(const DataFunction &function);
    void setHeaderFunction(const DataFunction &function);
    void setSeekFunction(const SeekFunction &function);

    // For the list of available get options and valid parameter types consult curl_easy_getinfo manual
    template <typename T> bool get(CURLINFO info, T *pointer) { return curl_easy_getinfo(handle_, info, pointer) == CURLE_OK; }
    template <typename T> T get(CURLINFO info);

    bool hasHttpHeader(const QString &header) const;
    void removeHttpHeader(const QString &header);

    QString httpHeader(const QString &header) const;
    void setHttpHeader(const QString &header, const QString &value);

    QByteArray httpHeaderRaw(const QString &header) const;
    void setHttpHeaderRaw(const QString &header, const QByteArray &encodedValue);

    CURL* handle() { return handle_; }
    void setPreferredMulti(CurlMulti *multi) { preferredMulti_ = multi; }

private slots:
    // Safety hack: substitute QObject's deleteLater whenever possible to make sure
    // that no callbacks will be called between deleteLater and curl handle removal.
    void deleteLater();

signals:
    void aborted();
    void progress(qint64 downloadTotal, qint64 downloadNow, qint64 uploadTotal, qint64 uploadNow);
    void done(CURLcode result);

protected:
    void removeFromMulti();
    void onCurlMessage(CURLMsg *message);
    void rebuildCurlHttpHeaders();

    static size_t staticCurlReadFunction(char *data, size_t size, size_t nitems, void *easyPtr);
    static size_t staticCurlWriteFunction(char *data, size_t size, size_t nitems, void *easyPtr);
    static size_t staticCurlHeaderFunction(char *data, size_t size, size_t nitems, void *easyPtr);
    static int staticCurlSeekFunction(void *easyPtr, curl_off_t offset, int origin);
    static int staticCurlXferInfoFunction(void *easyPtr, curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t uploadTotal, curl_off_t uploadNow);

    CURL            *handle_ = nullptr;
    CurlMulti       *preferredMulti_ = nullptr;
    CurlMulti       *runningOnMulti_ = nullptr;
    CURLcode        lastResult_ = CURLE_OK;
    DataFunction    readFunction_;
    DataFunction    writeFunction_;
    DataFunction    headerFunction_;
    SeekFunction    seekFunction_;

    bool                            httpHeadersWereSet_ = false;
    std::map<QString, QByteArray>   httpHeaders_;
    struct curl_slist*              curlHttpHeaders_ = nullptr;

    friend class CurlMulti;
};

template<typename T> T CurlEasy::get(CURLINFO info)
{
    T parameter;
    get(info, &parameter);
    return parameter;
}
