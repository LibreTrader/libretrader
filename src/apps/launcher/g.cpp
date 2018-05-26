#include "g.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include "utils.h"

QString G::brokerCtlAddr;
QString G::brokerPubAddr;
QString G::ctlAddr;
QString G::pubAddr;

nzmqt::ZMQContext* G::zctx;

QString G::configDir;
QString G::runDir;

QString G::pidFile;
void G::initialize()
{
    QString genConfDir = QStandardPaths::writableLocation(
                QStandardPaths::GenericConfigLocation);
    G::configDir = genConfDir + QDir::separator() + "libretrader";
    qDebug() << "configDir" << G::configDir;
    QDir(G::configDir).mkpath(".");

    QString genCacheDir = QStandardPaths::writableLocation(
                QStandardPaths::GenericCacheLocation);
    QString ltCacheDir = genCacheDir + QDir::separator() + "libretrader";
    // sockets and pid files in runDir
    G::runDir = ltCacheDir + QDir::separator() + "run";
    qDebug() << "runDir:" << G::runDir;
    QDir(G::runDir).mkpath(".");

    G::pidFile = G::runDir + QDir::separator() + "launcher.pid";

    // TODO: read settings from settings file

#ifdef Q_OS_WIN
    QList<quint16> ports = getFreePorts(2);
    G::brokerCtlAddr = QString("tcp://*:%1").arg(ports[0]);
    G::brokerPubAddr = QString("tcp://*:%1").arg(ports[1]);
    G::brokerCtlAddr = "tcp://*:30001";
    G::brokerPubAddr = "tcp://*:30002";
    qDebug().nospace() << "ctl: " << G::brokerCtlAddr << ", pub: "
                       << G::brokerPubAddr;
#else
    G::brokerCtlAddr = "ipc://" + G::runDir + "/lt-ctl";
    G::brokerPubAddr = "ipc://" + G::runDir + "/lt-pub";
#endif
    G::ctlAddr = "tcp://zmapi.org:10001";
    G::pubAddr = "tcp://zmapi.org:10002";
    //G::ctlAddr = "ipc:///tmp/ctl-hub";
    //G::pubAddr = "ipc:///tmp/pub-hub";
}
