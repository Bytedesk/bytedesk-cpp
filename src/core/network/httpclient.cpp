#include "httpclient.h"
#include <QFile>
#include <QHttpMultiPart>
#include <QSslConfiguration>
#include <QFileInfo>
#include <QJsonArray>

namespace Bytedesk {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeout(30000) // 默认30秒超时
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::setBaseUrl(const QString& baseUrl)
{
    m_baseUrl = baseUrl;
    if (!m_baseUrl.endsWith('/')) {
        m_baseUrl += '/';
    }
    qDebug() << "HTTP base URL set to:" << m_baseUrl;
}

void HttpClient::setAccessToken(const QString& token)
{
    m_accessToken = token;
    qDebug() << "Access token updated";
}

void HttpClient::clearAccessToken()
{
    m_accessToken.clear();
}

void HttpClient::get(const QString& path, const QUrlQuery& params,
                    HttpCallback onSuccess, HttpErrorCallback onError)
{
    QNetworkRequest request = createRequest(path, params);
    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        handleResponse(reply, onSuccess, onError);
        reply->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

void HttpClient::post(const QString& path, const QJsonObject& data,
                     HttpCallback onSuccess, HttpErrorCallback onError)
{
    QNetworkRequest request = createRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        handleResponse(reply, onSuccess, onError);
        reply->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

void HttpClient::put(const QString& path, const QJsonObject& data,
                    HttpCallback onSuccess, HttpErrorCallback onError)
{
    QNetworkRequest request = createRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_networkManager->put(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        handleResponse(reply, onSuccess, onError);
        reply->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

void HttpClient::deleteResource(const QString& path,
                               HttpCallback onSuccess, HttpErrorCallback onError)
{
    QNetworkRequest request = createRequest(path);
    QNetworkReply* reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        handleResponse(reply, onSuccess, onError);
        reply->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

void HttpClient::upload(const QString& path, const QString& fieldName,
                       const QString& filePath, const QJsonObject& metaData,
                       HttpCallback onSuccess, HttpErrorCallback onError)
{
    QNetworkRequest request = createRequest(path);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 添加文件
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QString error = QString("Failed to open file: %1").arg(filePath);
        qWarning() << error;
        if (onError) {
            onError(error);
        }
        delete file;
        delete multiPart;
        return;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QString("form-data; name=\"%1\"; filename=\"%2\"").arg(fieldName, fileName));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // 确保文件在multiPart删除时也被删除

    multiPart->append(filePart);

    // 添加元数据
    if (!metaData.isEmpty()) {
        QJsonDocument doc(metaData);
        QHttpPart metaPart;
        metaPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                          QString("form-data; name=\"metadata\""));
        metaPart.setBody(doc.toJson(QJsonDocument::Compact));
        multiPart->append(metaPart);
    }

    QNetworkReply* reply = m_networkManager->post(request, multiPart);

    connect(reply, &QNetworkReply::uploadProgress, this, &HttpClient::onUploadProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply, multiPart, onSuccess, onError]() {
        handleResponse(reply, onSuccess, onError);
        reply->deleteLater();
        multiPart->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

void HttpClient::download(const QString& path, const QString& savePath,
                         std::function<void(const QString&)> onSuccess,
                         HttpErrorCallback onError,
                         std::function<void(qint64, qint64)> onProgress)
{
    QNetworkRequest request = createRequest(path);
    QNetworkReply* reply = m_networkManager->get(request);

    DownloadInfo info;
    info.savePath = savePath;
    info.file = new QFile(savePath);
    info.onSuccess = onSuccess;
    info.onError = onError;
    info.onProgress = onProgress;

    if (!info.file->open(QIODevice::WriteOnly)) {
        QString error = QString("Failed to create file: %1").arg(savePath);
        qWarning() << error;
        if (onError) {
            onError(error);
        }
        delete info.file;
        reply->deleteLater();
        return;
    }

    m_downloads[reply] = info;

    connect(reply, &QNetworkReply::downloadProgress, this, &HttpClient::onDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (m_downloads.contains(reply)) {
            m_downloads[reply].file->write(reply->readAll());
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (m_downloads.contains(reply)) {
            DownloadInfo& info = m_downloads[reply];
            info.file->close();

            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "Download completed:" << info.savePath;
                if (info.onSuccess) {
                    info.onSuccess(info.savePath);
                }
            } else {
                info.file->remove(); // 删除不完整的文件
                QString error = QString("Download failed: %1").arg(reply->errorString());
                qWarning() << error;
                if (info.onError) {
                    info.onError(error);
                }
            }

            delete info.file;
            m_downloads.remove(reply);
        }
        reply->deleteLater();
    });

    emit requestStarted(request.url().toString());
}

QNetworkRequest HttpClient::createRequest(const QString& path, const QUrlQuery& params)
{
    QString fullUrl = getFullUrl(path, params);
    QUrl url(fullUrl);
    QNetworkRequest request(url);

    // 设置通用headers
    request.setHeader(QNetworkRequest::UserAgentHeader, "Bytedesk-Qt/1.0");
    request.setRawHeader("Accept", "application/json");

    // 添加认证token
    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    }

    // SSL配置
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 生产环境应该验证证书
    request.setSslConfiguration(sslConfig);

    return request;
}

void HttpClient::handleResponse(QNetworkReply* reply, HttpCallback onSuccess, HttpErrorCallback onError)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    QString url = reply->request().url().toString();

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "HTTP success:" << statusCode << url;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                QJsonObject response = doc.object();
                if (onSuccess) {
                    onSuccess(response);
                }
            } else if (doc.isArray()) {
                QJsonObject response;
                response["data"] = doc.array();
                if (onSuccess) {
                    onSuccess(response);
                }
            } else {
                // 空响应或非JSON响应
                QJsonObject response;
                response["statusCode"] = statusCode;
                if (onSuccess) {
                    onSuccess(response);
                }
            }
        } else {
            QString error = QString("Failed to parse response: %1").arg(parseError.errorString());
            qWarning() << error;
            if (onError) {
                onError(error);
            }
        }

        emit requestFinished(url, true);
    } else {
        QString error;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject jsonResponse = doc.object();
            if (jsonResponse.contains("message")) {
                error = jsonResponse["message"].toString();
            } else if (jsonResponse.contains("error")) {
                error = jsonResponse["error"].toString();
            } else {
                error = reply->errorString();
            }
        } else {
            error = reply->errorString();
        }

        qWarning() << "HTTP error:" << statusCode << error << url;
        if (onError) {
            onError(error);
        }

        emit requestFinished(url, false);
        emit networkErrorOccurred(error);
    }
}

QString HttpClient::getFullUrl(const QString& path, const QUrlQuery& params)
{
    QString url = m_baseUrl + path;

    if (!params.isEmpty()) {
        url += "?" + params.toString(QUrl::FullyEncoded);
    }

    return url;
}

void HttpClient::onReplyFinished()
{
    // 已在handleResponse中处理
}

void HttpClient::onReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    // 已在handleResponse中处理
}

void HttpClient::onSslErrors(const QList<QSslError>& errors)
{
    qWarning() << "SSL errors:" << errors;
    // 生产环境应该正确处理SSL错误
}

void HttpClient::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_UNUSED(bytesSent);
    Q_UNUSED(bytesTotal);
    qDebug() << "Upload progress:" << bytesSent << "/" << bytesTotal;
}

void HttpClient::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && m_downloads.contains(reply)) {
        DownloadInfo& info = m_downloads[reply];
        if (info.onProgress) {
            info.onProgress(bytesReceived, bytesTotal);
        }
    }
}

} // namespace Bytedesk
