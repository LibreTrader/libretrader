#include "g.h"

#include <QStandardPaths>
#include <QDir>

nzmqt::ZMQContext *G::zctx;

nzmqt::ZMQSocket *G::sockDealer;
nzmqt::ZMQSocket *G::sockBrokerSub;
nzmqt::ZMQSocket *G::sockSub;

ReturningDealer *G::dealer;
Subscriber *G::subscriber;

QQmlContext *G::qctx;
QQmlEngine *G::qmlEng;

InstrumentsModel *G::instrumentsModel;

QString G::runDir;

void G::initialize()
{
    // TODO: read settings from settings file

    QString genCacheDir = QStandardPaths::writableLocation(
                QStandardPaths::GenericCacheLocation);
    QString ltCacheDir = genCacheDir + QDir::separator() + "libretrader";
    G::runDir = ltCacheDir + QDir::separator() + "run";
    QDir(G::runDir).mkpath(".");
}
