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
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QIcon>

#include "backward.hpp"
#include "nzmqt/nzmqt.hpp"
#include "logging.h"
#include "utils.h"
#include "returningdealer.h"
#include "dommodel.h"
#include "g.h"
#include "subscriber.h"

bool reuseFunnel;

QString funnelCtlAddr;
QString funnelPubAddr;
QString ctlAddr;
QString pubAddr;
QString brokerPubAddr;
QString instrumentsFile;

void parseArgs(QGuiApplication& app)
{
    QTextStream err(stderr);

    QCommandLineParser parser;
    parser.setApplicationDescription("LibreTrader Dom");
    parser.addHelpOption();
    parser.addPositionalArgument("instruments-file",
                                 "path to instruments file");
    parser.addPositionalArgument("ctl-addr",
                                 "address of upstream CTL socket");
    parser.addPositionalArgument("pub-addr",
                                 "address of upstream PUB socket");

    parser.addOptions({
        {"broker-pub", "address of message broker socket", "addr"},
        {"reuse-funnel", "reuse old mdfunnel"},
        {"funnel-ctl", "address of funnel CTL socket", "addr"},
        {"funnel-pub", "address of funnel PUB socket", "addr"},
    });
    parser.process(app);

    QStringList posArgs = parser.positionalArguments();
    instrumentsFile = posArgs[0];
    ctlAddr = posArgs[1];
    pubAddr = posArgs[2];
    brokerPubAddr = parser.value("broker-pub");
    reuseFunnel = parser.isSet("reuse-funnel");
    if (reuseFunnel) {
        funnelCtlAddr = parser.value("funnel-ctl");
        funnelPubAddr = parser.value("funnel-pub");
        if (funnelCtlAddr.isEmpty() || funnelPubAddr.isEmpty()) {
            err << "'funnel-ctl' and 'funnel-pub' must be provided "
                << "when using 'reuse-funnel' option";
            err.flush();
            exit(1);
        }
    }
}

void launchFunnel()
{
    QString uuid;
#ifdef Q_OS_WIN
    QList<quint16> ports = getFreePorts(2);
    QString funnelCtlAddrBind = QString("tcp://*:%1").arg(ports[0]);
    funnelCtlAddr = QString(funnelCtlAddrBind).replace(
                QRegularExpression("tcp://.*:"), "tcp://localhost:");
    QString funnelPubAddrBind = QString("tcp://*:%1").arg(ports[1]);
    funnelPubAddr = QString(funnelPubAddrBind).replace(
                QRegularExpression("tcp://.*:"), "tcp://localhost:");
#else
    uuid = QUuid::createUuid().toString();
    uuid = uuid.mid(1, uuid.length() - 2);
    QString funnelCtlAddrBind = "ipc://" + G::runDir + "/ctl-funnel-" + uuid;
    funnelCtlAddr = funnelCtlAddrBind;
    uuid = QUuid::createUuid().toString();
    uuid = uuid.mid(1, uuid.length() - 2);
    QString funnelPubAddrBind = "ipc://" + G::runDir + "/pub-funnel-" + uuid;
    funnelPubAddr = funnelPubAddrBind;
#endif

    QProcess funnelProcess;

    funnelProcess.setProgram("./mdfunnel");
    funnelProcess.setArguments({
        ctlAddr,
        funnelCtlAddrBind,
        pubAddr,
        funnelPubAddrBind,
        "--log-level",
        "DEBUG",
    });
    qDebug() << funnelProcess.arguments();

    QString logFile = QStandardPaths::writableLocation(
                QStandardPaths::TempLocation) + QDir::separator();
    logFile += QString("mdfunnel-%1.log")
            .arg(QCoreApplication::applicationPid());
    funnelProcess.setProcessChannelMode(QProcess::MergedChannels);
    funnelProcess.setStandardOutputFile(logFile);
    qDebug() << "mdfunnel logfile:" << logFile;

#ifdef Q_OS_WIN
    // mdfunnel is not listening WM_CLOSE event
    launchManagedChildProcessDetached(funnelProcess, true);
#else
    launchManagedChildProcessDetached(funnelProcess, false);
#endif
}

void setupFunnel()
{
    if (!reuseFunnel) {
        launchFunnel();
    } else {
        qDebug() << "reusing mdfunnel:"
                 << funnelCtlAddr << "/" << funnelPubAddr;
    }
}

void initZMQSockets()
{
    G::zctx = nzmqt::createDefaultContext();

    G::sockDealer = G::zctx->createSocket(nzmqt::ZMQSocket::TYP_DEALER);
    G::sockDealer->connectTo(funnelCtlAddr);
    G::sockDealer->setLinger(0);

    G::sockSub = G::zctx->createSocket(nzmqt::ZMQSocket::TYP_SUB);
    G::sockSub->connectTo(funnelPubAddr);
    G::sockSub->subscribeTo("");
    G::sockSub->setLinger(0);

    G::dealer = new ReturningDealer(G::sockDealer, G::qmlEng);
    G::qctx->setContextProperty("dealer", G::dealer);

    if (!brokerPubAddr.isEmpty())
    {
        G::sockBrokerSub = G::zctx->createSocket(nzmqt::ZMQSocket::TYP_SUB);
        try {
            G::sockBrokerSub->connectTo(brokerPubAddr);
        } catch (const zmq::error_t&)
        {
            // ignore error on connecting ...
        }
        G::sockBrokerSub->subscribeTo("instruments_list");
        G::sockBrokerSub->setLinger(0);
    } else {
        G::sockBrokerSub = nullptr;
    }
}

void brokerSubMessageReceived(const QList<QByteArray> &msg) {
    QString topic(msg[0]);
    if (topic != "instruments_list")
        return;
    qDebug() << "instruments_list received";
    G::instrumentsModel->loadFromBytes(msg[1]);
}

void setupBrokerSub() {
    if (G::sockBrokerSub) {
        QObject::connect(G::sockBrokerSub,
                         &nzmqt::ZMQSocket::messageReceived,
                         &brokerSubMessageReceived);
    }
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    backward::SignalHandling sh;
#endif

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/icon_dom.png"));

    parseArgs(app);

    setupBasicConsoleLogging();

    G::initialize();

    QQmlApplicationEngine engine;
    G::qmlEng = &engine;
    G::qctx = G::qmlEng->rootContext();

    setupFunnel();

    initZMQSockets();

    DomModel domModel;
    G::qctx->setContextProperty("domModel", &domModel);

    G::instrumentsModel = new InstrumentsModel();
    G::instrumentsModel->loadFromFile(instrumentsFile);
    G::qctx->setContextProperty("instrumentsModel", G::instrumentsModel);

    setupBrokerSub();

    G::subscriber = new Subscriber(&domModel);
    G::qctx->setContextProperty("subscriber", G::subscriber);

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // waiting for SUB socket to get ready before we can continue

    auto sockPubRequester = [&]() {
        QJsonObject msg;
        QJsonObject header;
        header["MsgType"] = "1";  // TestRequest
        msg["Header"] = header;
        QJsonObject body;
        body["ZMSendToPub"] = true;
        msg["Body"] = body;
        G::dealer->send(msg);
    };

    QTimer pubReqTimer;
    // run every 0.5 seconds
    pubReqTimer.setInterval(100);
    // singleshot ok?
    //pubReqTimer.setSingleShot(true);
    QObject::connect(&pubReqTimer,
                     &QTimer::timeout,
                     sockPubRequester);
    pubReqTimer.start();

    QMetaObject::Connection connection;

    auto pubMessageReceived = [&]
            (const QList<QByteArray>) {

        qDebug() << "pub socket is ready";

        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty())
            exit(1);

        // "Unclog" the dealer socket ... otherwise underlying QSocketNotifier
        // of sockDealer may not fire signals ... Really strange.
        while (!G::sockDealer->receiveMessage().isEmpty()) { }

        QObject::disconnect(connection);
        pubReqTimer.stop();
    };

    connection = QObject::connect(G::sockSub,
                                  &nzmqt::ZMQSocket::messageReceived,
                                  pubMessageReceived);

    qDebug() << "main thread:" << QThread::currentThread();
    qDebug() << "sock: " << G::sockDealer;

    qDebug() << "starting event loop ...";
    int exitCode = app.exec();

    qDebug() << "deleting zmq context ...";
    delete G::zctx;

    return exitCode;
}
