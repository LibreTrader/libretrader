#ifndef MESSAGEBROKER_H
#define MESSAGEBROKER_H

#include <functional>

#include <QObject>
#include <QHash>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

#include "nzmqt/nzmqt.hpp"

class CTLMessage;

class MessageBroker : public QObject
{
    Q_OBJECT

public:
    explicit MessageBroker(QObject *parent = nullptr);

private:

    // void handleMessage(QByteArray ident, QByteArray
    void messageReceived(const QList<QByteArray> &msgParts);

    QJsonObject handleBroadcast(const CTLMessage& msg);

    nzmqt::ZMQSocket *router;
    nzmqt::ZMQSocket *pub;

};

#endif // MESSAGEBROKER_H
