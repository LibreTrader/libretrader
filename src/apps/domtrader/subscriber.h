#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QJsonObject>
#include <QJSValue>

#include "dommodel.h"


class Subscriber : public QObject
{
    Q_OBJECT

public:

    explicit Subscriber(DomModel *model, QObject *parent = nullptr);

    Q_INVOKABLE void setSubscription(int index);
    Q_INVOKABLE void unsubscribeAndFinish(QJSValue finished);

private:

    void marketDataReceived(const QList<QByteArray> &msgParts);
    void subscribe(QJsonObject ins);
    void unsubscribe(QJsonObject ins);

    DomModel *_model;
    QJsonObject _prevIns;

};

#endif // SUBSCRIBER_H
