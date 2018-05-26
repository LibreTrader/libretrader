#include <functional>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include <QProcess>
#include <QUuid>
#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <QThread>
#include <QJsonDocument>
#include <QLockFile>
#include <QDebug>
#include <QIcon>

#include "treemodel.h"
#include "backward.hpp"
#include "nzmqt/nzmqt.hpp"
#include "logging.h"
#include "utils.h"
#include "g.h"

QString ctlAddr;
QString brokerCtlAddr;
QLockFile *lockFile;

void parseArgs(QGuiApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("LibreTrader instrument selector");
    parser.addHelpOption();
    parser.addPositionalArgument("instruments-file",
                                 "path to instruments file");
    parser.addPositionalArgument("ctl-addr",
                                 "address of upstream CTL socket");

    parser.addOptions({
        {"broker-ctl", "address of message broker socket", "addr"},
    });
    parser.process(app);

    QStringList posArgs = parser.positionalArguments();
    G::instrumentsFile = posArgs[0];
    ctlAddr = posArgs[1];

    brokerCtlAddr = parser.value("broker-ctl");

    qDebug() << brokerCtlAddr << ctlAddr << G::instrumentsFile;
}

void initZMQSockets()
{
    G::zctx = nzmqt::createDefaultContext();

    G::sockDealer = G::zctx->createSocket(nzmqt::ZMQSocket::TYP_DEALER);
    G::sockDealer->connectTo(ctlAddr);
    G::sockDealer->setLinger(0);
    G::dealer = new ReturningDealer(G::sockDealer, G::qmlEng);
    G::qctx->setContextProperty("dealer", G::dealer);

    if (!brokerCtlAddr.isEmpty())
    {
        qDebug() << "broker-ctl:" << brokerCtlAddr;
        G::sockBrokerDealer =
                G::zctx->createSocket(nzmqt::ZMQSocket::TYP_DEALER);
        G::sockBrokerDealer->connectTo(brokerCtlAddr);
        G::sockBrokerDealer->setLinger(0);
        G::brokerDealer = new ReturningDealer(G::sockBrokerDealer, G::qmlEng);
    } else {
        G::sockBrokerDealer = nullptr;
        G::brokerDealer = nullptr;
    }
}

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

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/icon_ins-sel.png"));

    parseArgs(app);

    setupBasicConsoleLogging();

    G::initialize();

    checkOnlyInstance();

    QQmlApplicationEngine engine;
    G::qmlEng = &engine;
    G::qctx = G::qmlEng->rootContext();

    initZMQSockets();

    G::instrumentsModel = new InstrumentsModel();
    G::instrumentsModel->loadFromFile(G::instrumentsFile);
    G::qctx->setContextProperty("instrumentsModel", G::instrumentsModel);

    TreeModel treeModel;
    G::qctx->setContextProperty("treeModel", &treeModel);

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        exit(1);

    int exitCode = app.exec();

    delete lockFile;  // delete the pidfile

    qDebug() << "deleting zmq context ...";
    delete G::zctx;

    return exitCode;
}
