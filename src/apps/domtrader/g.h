#ifndef G_H
#define G_H

#include <QString>
#include <QQmlContext>
#include <QQmlEngine>

#include <nzmqt/nzmqt.hpp>
#include "returningdealer.h"
#include "subscriber.h"
#include "instrumentsmodel.h"

class G
{
public:

    static nzmqt::ZMQContext *zctx;

    static nzmqt::ZMQSocket *sockDealer;
    static nzmqt::ZMQSocket *sockBrokerSub;
    static nzmqt::ZMQSocket *sockSub;

    static ReturningDealer *dealer;
    static Subscriber *subscriber;

    static QQmlContext *qctx;
    static QQmlEngine *qmlEng;

    static InstrumentsModel *instrumentsModel;

    static QString runDir;

    static void initialize();

private:
    explicit G() {}

};

#endif // G_H
