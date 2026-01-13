#ifndef APIBASE_H
#define APIBASE_H

#include <QObject>
#include "httpclient.h"
#include "models/user.h"

namespace Bytedesk {

// API基础类 - 所有API服务类的基类
class ApiBase : public QObject
{
    Q_OBJECT

public:
    explicit ApiBase(HttpClient* httpClient, QObject* parent = nullptr);
    virtual ~ApiBase();

protected:
    HttpClient* httpClient() const { return m_httpManager; }

    // 检查响应状态
    bool isResponseSuccess(const QJsonObject& response) const;
    QString getResponseMessage(const QJsonObject& response) const;
    QJsonObject getResponseData(const QJsonObject& response) const;

    // 错误处理
    void handleApiError(const QString& error);
    void handleNetworkError(const QString& error);

    // 参数验证
    bool validateRequired(const QJsonObject& data, const QStringList& requiredFields, QString& error) const;

private:
    HttpClient* m_httpManager;
};

} // namespace Bytedesk

#endif // APIBASE_H
