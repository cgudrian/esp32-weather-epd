#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "displayimageprovider.h"
#include "quantities.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QLocale::setDefault(QLocale("de_DE"));

    QQmlApplicationEngine engine;

    engine.addImageProvider(QLatin1String("display"), new DisplayImageProvider);

    const QUrl url(u"qrc:/WeatherStation/Main.qml"_qs);
    qDebug() << url;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
