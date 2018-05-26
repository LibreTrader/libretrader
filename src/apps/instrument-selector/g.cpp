#include "g.h"

#include <QStandardPaths>
#include <QDir>
#include <QThread>
#include <QDebug>

#include "utils.h"

nzmqt::ZMQContext *G::zctx;

nzmqt::ZMQSocket *G::sockDealer;
nzmqt::ZMQSocket *G::sockBrokerDealer;
nzmqt::ZMQSocket *G::sockSub;

ReturningDealer *G::dealer;
ReturningDealer *G::brokerDealer;

QQmlContext *G::qctx;
QQmlEngine *G::qmlEng;

InstrumentsModel *G::instrumentsModel;

QString G::instrumentsFile;
QString G::runDir;
QString G::pidFile;

void G::initialize()
{
    QString genCacheDir = QStandardPaths::writableLocation(
                QStandardPaths::GenericCacheLocation);
    QString ltCacheDir = genCacheDir + QDir::separator() + "libretrader";
    G::runDir = ltCacheDir + QDir::separator() + "run";
    qDebug() << "runDir:" << G::runDir;
    QDir(G::runDir).mkpath(".");
    G::pidFile = G::runDir + QDir::separator() + "instrument-selector.pid";
}
