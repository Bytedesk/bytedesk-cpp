#include "authapi.h"

namespace Bytedesk {

AuthApi::AuthApi(HttpClient* httpClient, QObject* parent)
    : ApiBase(httpClient, parent)
{
}

AuthApi::~AuthApi()
{
}

void AuthApi::login(const LoginRequest& request, LoginCallback callback)
{
    qDebug() << "Login request for user:" << request.username;

    httpClient()->post(m_authPath + "/login", request.toJson(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                LoginResult result = LoginResult::fromJson(response);
                qDebug() << "Login successful:" << result.user.getUid();

                // 更新HTTP客户端的token
                if (!result.accessToken.isEmpty()) {
                    httpClient()->setAccessToken(result.accessToken);
                }

                emit loginCompleted(result);

                if (callback) {
                    callback(result);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Login failed:" << message;

                LoginResult result;
                result.success = false;
                result.message = message;

                if (callback) {
                    callback(result);
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Login network error:" << error;

            LoginResult result;
            result.success = false;
            result.message = error;

            handleNetworkError(error);

            if (callback) {
                callback(result);
            }
        }
    );
}

void AuthApi::registerUser(const RegisterRequest& request, RegisterCallback callback)
{
    qDebug() << "Register request for user:" << request.username;

    httpClient()->post(m_authPath + "/register", request.toJson(),
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);
            QString message = getResponseMessage(response);

            if (success) {
                qDebug() << "Registration successful";
            } else {
                qWarning() << "Registration failed:" << message;
            }

            if (callback) {
                callback(success, message);
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Registration network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(false, error);
            }
        }
    );
}

void AuthApi::logout(LogoutCallback callback)
{
    qDebug() << "Logout request";

    httpClient()->post(m_authPath + "/logout", QJsonObject(),
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Logout successful";
                httpClient()->clearAccessToken();
                emit logoutCompleted();
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Logout failed:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Logout network error:" << error;
            handleNetworkError(error);

            // 即使网络失败也清除本地token
            httpClient()->clearAccessToken();

            if (callback) {
                callback(false);
            }
        }
    );
}

void AuthApi::refreshAccessToken(const QString& refreshToken, RefreshTokenCallback callback)
{
    qDebug() << "Refresh access token";

    QJsonObject request;
    request["refreshToken"] = refreshToken;

    httpClient()->post(m_authPath + "/refresh", request,
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                QString accessToken = data["accessToken"].toString();

                if (!accessToken.isEmpty()) {
                    qDebug() << "Token refreshed successfully";
                    httpClient()->setAccessToken(accessToken);

                    if (callback) {
                        callback(accessToken);
                    }
                } else {
                    qWarning() << "Refresh response missing access token";
                    if (callback) {
                        callback(QString());
                    }
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Token refresh failed:" << message;
                if (callback) {
                    callback(QString());
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Token refresh network error:" << error;
            handleNetworkError(error);
            if (callback) {
                callback(QString());
            }
        }
    );
}

void AuthApi::getCurrentUser(std::function<void(const User&)> callback,
                             std::function<void(const QString&)> onError)
{
    qDebug() << "Get current user info";

    httpClient()->get(m_userPath + "/current", QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                User user = User::fromJson(data);

                qDebug() << "Got user info:" << user.getUid();

                if (callback) {
                    callback(user);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get user info:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get user info network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void AuthApi::updateProfile(const QString& nickname, const QString& avatar,
                           std::function<void(bool)> callback,
                           std::function<void(const QString&)> onError)
{
    qDebug() << "Update profile:" << nickname;

    QJsonObject request;
    if (!nickname.isEmpty()) request["nickname"] = nickname;
    if (!avatar.isEmpty()) request["avatar"] = avatar;

    httpClient()->put(m_userPath + "/profile", request,
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Profile updated successfully";
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to update profile:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Update profile network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

} // namespace Bytedesk
