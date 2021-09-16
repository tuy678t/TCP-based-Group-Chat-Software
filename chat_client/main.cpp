#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "TcpClient.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<TcpClient>("DataModel", 1, 0, "TcpClient");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/QML/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
