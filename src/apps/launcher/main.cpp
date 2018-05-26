#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QLockFile>
#include <QIcon>

#include "g.h"
#include "backward.hpp"
#include "nzmqt/nzmqt.hpp"
#include "logging.h"
#include "messagebroker.h"
#include "applauncher.h"
#include "utils.h"

QLockFile *lockFile;

void checkOnlyInstance() {

    lockFile = new QLockFile(G::pidFile);
    lockFile->setStaleLockTime(0);
    if (!lockFile->tryLock()) {
        // exit the application with error status
        qCritical() << "another instance is already running";
        exit(1);
    }
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    backward::SignalHandling sh;
#endif    

    setupBasicConsoleLogging();

    G::initialize();

    checkOnlyInstance();

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/icon_lt.png"));

    G::zctx = nzmqt::createDefaultContext();

    MessageBroker broker;

    AppLauncher appLauncher;

    QQmlApplicationEngine engine;
    QQmlContext *qctx = engine.rootContext();

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    qctx->setContextProperty("appLauncher", &appLauncher);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int exitCode = app.exec();

    qDebug() << "deleting zmq context ...";
    delete G::zctx;

    delete lockFile;

    return exitCode;
}
