#ifndef G_H
#define G_H

#include <QString>
#include <QQmlContext>
#include <QQmlEngine>

#include <nzmqt/nzmqt.hpp>
#include "returningdealer.h"
#include "instrumentsmodel.h"

class G
{
public:

    static nzmqt::ZMQContext *zctx;

    static nzmqt::ZMQSocket *sockDealer;
    static nzmqt::ZMQSocket *sockBrokerDealer;
    static nzmqt::ZMQSocket *sockSub;

    static ReturningDealer *dealer;
    static ReturningDealer *brokerDealer;

    static QQmlContext *qctx;
    static QQmlEngine *qmlEng;

    static InstrumentsModel *instrumentsModel;

    static QString instrumentsFile;
    static QString runDir;
    static QString pidFile;

    static void initialize();

private:
    explicit G() {}

};

#endif // G_H
