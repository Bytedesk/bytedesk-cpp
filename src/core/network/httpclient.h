#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QFile>
#include <QHash>
#include <functional>

namespace Bytedesk {

// HTTP响应回调
using HttpCallback = std::function<void(const QJsonObject& response)>;
using HttpErrorCallback = std::function<void(const QString& error)>;

// HTTP客户端类
class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient();

    // 设置基础URL
    void setBaseUrl(const QString& baseUrl);
    QString getBaseUrl() const { return m_baseUrl; }

    // 设置认证token
    void setAccessToken(const QString& token);
    void clearAccessToken();

    // GET请求
    void get(const QString& path, const QUrlQuery& params = QUrlQuery(),
            HttpCallback onSuccess = nullptr, HttpErrorCallback onError = nullptr);

    // POST请求
    void post(const QString& path, const QJsonObject& data,
             HttpCallback onSuccess = nullptr, HttpErrorCallback onError = nullptr);

    // PUT请求
    void put(const QString& path, const QJsonObject& data,
            HttpCallback onSuccess = nullptr, HttpErrorCallback onError = nullptr);

    // DELETE请求
    void deleteResource(const QString& path,
                       HttpCallback onSuccess = nullptr, HttpErrorCallback onError = nullptr);

    // 上传文件
    void upload(const QString& path, const QString& fieldName,
               const QString& filePath, const QJsonObject& metaData = QJsonObject(),
               HttpCallback onSuccess = nullptr, HttpErrorCallback onError = nullptr);

    // 下载文件
    void download(const QString& path, const QString& savePath,
                 std::function<void(const QString& filePath)> onSuccess = nullptr,
                 HttpErrorCallback onError = nullptr,
                 std::function<void(qint64 bytesReceived, qint64 bytesTotal)> onProgress = nullptr);

    // 设置超时
    void setTimeout(int milliseconds) { m_timeout = milliseconds; }

signals:
    void requestStarted(const QString& url);
    void requestFinished(const QString& url, bool success);
    void networkErrorOccurred(const QString& error);

private slots:
    void onReplyFinished();
    void onReplyError(QNetworkReply::NetworkError error);
    void onSslErrors(const QList<QSslError>& errors);
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QNetworkRequest createRequest(const QString& path, const QUrlQuery& params = QUrlQuery());
    void handleResponse(QNetworkReply* reply, HttpCallback onSuccess, HttpErrorCallback onError);
    QString getFullUrl(const QString& path, const QUrlQuery& params = QUrlQuery());

    QNetworkAccessManager* m_networkManager;
    QString m_baseUrl;
    QString m_accessToken;
    int m_timeout;

    // 下载相关
    struct DownloadInfo {
        QString savePath;
        QFile* file;
        std::function<void(const QString&)> onSuccess;
        HttpErrorCallback onError;
        std::function<void(qint64, qint64)> onProgress;
    };
    QHash<QNetworkReply*, DownloadInfo> m_downloads;
};

} // namespace Bytedesk

#endif // HTTPCLIENT_H
