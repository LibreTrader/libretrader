#include "messagebroker.h"

#include <functional>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QHash>

#include "g.h"
#include "utils.h"
#include "ctlmessage.h"

using namespace nzmqt;

MessageBroker::MessageBroker(QObject *parent)
    : QObject(parent)
{
    router = G::zctx->createSocket(ZMQSocket::TYP_ROUTER);
    router->bindTo(G::brokerCtlAddr);
    router->setLinger(0);
    QObject::connect(router, &ZMQSocket::messageReceived,
                     this, &MessageBroker::messageReceived);

    pub = G::zctx->createSocket(ZMQSocket::TYP_PUB);
    pub->bindTo(G::brokerPubAddr);
    pub->setLinger(0);
}

void MessageBroker::messageReceived(const QList<QByteArray> &msgParts)
{

    CTLMessage msg;
    QJsonObject res;

    try {
        msg = CTLMessage(msgParts);
    } catch (const Exception &err) {
        qCritical() << "[broker]" << err;
        return;
    }

    QString msgType;

    try {

        msgType = msg.payload["msg_type"].toString();
        if (msgType.isEmpty())
            throw ArgumentException("\"msg_type\" field missing");
        qDebug().nospace()
                << "[broker] > "
                << "ident=" << msg.ident
                << ", msg_type=" << msgType
                << ", msg_id=" << msg.msgID;

        if (msgType == "broadcast") {
            res["body"] = handleBroadcast(msg);
        } else {
            throw ArgumentException("msg_type not recognized");
        }
        res["error"] = false;

    } catch (const Exception &err) {
        qCritical() << "[broker] " << err;
        res["error"] = true;
        QJsonObject body;
        body["msg"] = err.toString();
        res["body"] = body;
    }

    if (!res.isEmpty()) {

        QList<QByteArray> msgOut;
        msgOut << msg.ident << "" << QJsonDocument(res).toJson();
        router->sendMessage(msgOut);

        qDebug().nospace()
                << "[broker] < "
                << "ident=" << msg.ident
                << ", msg_type=" << msgType
                << ", msg_id=" << msg.msgID;
    }

}

QJsonObject MessageBroker::handleBroadcast(const CTLMessage& msg)
{
    QJsonObject body = msg.payload["body"].toObject();
    if (body.isEmpty())
        throw ArgumentException("\"body\" field missing or empty");
    QString topic = body["topic"].toString();
    if (topic.isEmpty())
        throw ArgumentException("\"topic\" field missing from body");
    QString payload = body["data"].toString();
    QList<QByteArray> msgOut;
    msgOut << topic.toUtf8() << payload.toUtf8();
    pub->sendMessage(msgOut);
    return QJsonObject();
}
