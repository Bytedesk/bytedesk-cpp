#include "apibase.h"
#include <QDebug>

namespace Bytedesk {

ApiBase::ApiBase(HttpClient* httpClient, QObject* parent)
    : QObject(parent)
    , m_httpManager(httpClient)
{
    Q_ASSERT(httpClient);
}

ApiBase::~ApiBase()
{
}

bool ApiBase::isResponseSuccess(const QJsonObject& response) const
{
    // 假设API响应格式为: { "statusCode": 200, "message": "success", "data": {} }
    if (response.contains("statusCode")) {
        int statusCode = response["statusCode"].toInt();
        return statusCode >= 200 && statusCode < 300;
    }
    return false;
}

QString ApiBase::getResponseMessage(const QJsonObject& response) const
{
    if (response.contains("message")) {
        return response["message"].toString();
    }
    return QString();
}

QJsonObject ApiBase::getResponseData(const QJsonObject& response) const
{
    if (response.contains("data")) {
        QJsonValue dataValue = response["data"];
        if (dataValue.isObject()) {
            return dataValue.toObject();
        }
    }
    return QJsonObject();
}

void ApiBase::handleApiError(const QString& error)
{
    qWarning() << "API Error:" << error;
    // 可以在这里添加统一的错误处理逻辑，如显示通知
}

void ApiBase::handleNetworkError(const QString& error)
{
    qWarning() << "Network Error:" << error;
    // 可以在这里添加统一的网络错误处理逻辑
}

bool ApiBase::validateRequired(const QJsonObject& data, const QStringList& requiredFields, QString& error) const
{
    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            error = QString("Missing required field: %1").arg(field);
            return false;
        }

        QJsonValue value = data[field];
        if (value.isUndefined() || value.isNull()) {
            error = QString("Field '%1' cannot be null").arg(field);
            return false;
        }

        if (value.isString() && value.toString().isEmpty()) {
            error = QString("Field '%1' cannot be empty").arg(field);
            return false;
        }
    }
    return true;
}

} // namespace Bytedesk
