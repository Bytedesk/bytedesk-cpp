#include "ui/mainwindow.h"
#include "models/config.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用信息
    a.setApplicationName("ByteDesk Qt");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("ByteDesk");

    // 加载翻译
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qt_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // 初始化配置
    qDebug() << "ByteDesk Qt Client starting...";
    qDebug() << "API URL:" << BYTDESK_CONFIG->getApiUrl();
    qDebug() << "MQTT URL:" << BYTDESK_CONFIG->getMqttUrl();

    // 显示主窗口
    MainWindow w;
    w.show();

    return a.exec();
}
