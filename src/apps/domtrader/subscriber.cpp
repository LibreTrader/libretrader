#include "subscriber.h"

#include <QDebug>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>

#include "g.h"


Subscriber::Subscriber(DomModel *model, QObject *parent) : QObject(parent) {
    _model = model;
    connect(G::sockSub,
            &nzmqt::ZMQSocket::messageReceived,
            this,
            &Subscriber::marketDataReceived);
}

void Subscriber::setSubscription(int index) {

    QJsonObject newIns;
    if (index >= 0 && index < G::instrumentsModel->rowCount(QModelIndex())) {
        newIns = G::instrumentsModel->at(index);
    }
    if (newIns == _prevIns) {
        qDebug() << "setSubscription: newIns == prevIns";
        return;
    }
    unsubscribe(_prevIns);
    subscribe(newIns);
    _prevIns = newIns;
}

void Subscriber::unsubscribeAndFinish(QJSValue finished)
{
    QJsonObject ins = _prevIns;

    if (ins.isEmpty())
        return;

    qDebug() << "unsubscribing from" << ins["Name"] << "...";

    auto marketDataReqResponse = [=](QJsonObject) mutable {
        qDebug() << "unsubscribed1!";
        finished.call();
    };

    QJsonObject msg;
    QJsonObject header;
    header["MsgType"] = "V";  // MarketDataRequest
    header["ZMEndpoint"] = ins["ZMEndpoint"];
    msg["Header"] = header;
    QJsonObject body;
    body["ZMInstrumentID"] = ins["ZMInstrumentID"];
    body["SubscriptionRequestType"] = "2"; // unsubscribe
    body["MDReqGrp"] = "*"; // all MDEntryType
    body["MDReqID"] = "domtrader";
    msg["Body"] = body;
    G::dealer->send(msg, marketDataReqResponse);
}

void Subscriber::marketDataReceived(const QList<QByteArray> &msgParts) {
    QByteArray topic = msgParts[0];
    QString msgType = topic.split(0)[0];
    qDebug() << "MsgType received:" << msgType;
    QJsonObject msg = QJsonDocument::fromJson(msgParts[1]).object();
    if (msgType == "X") {  // MarketDataIncrementalUpdate
        _model->updateIncremental(msg);
    } else if (msgType == "W") {  // MarketDataSnapshotFullRefresh
        _model->updateSnapshot(msg);
    }
}

void Subscriber::subscribe(QJsonObject ins)
{
    if (ins.isEmpty())
        return;

    qDebug() << "subscribing to" << ins["Name"] << "...";

    auto marketDataReqResponse = [=](QJsonObject msg) {
        if (msg["Header"].toObject()["MsgType"].toString() == "ZMM") {
            // ZMMarketDataRequestResponse
            qDebug() << "subscribed" << ins["Name"];
        }
        qDebug() << msg;
    };

    _model->setMinPriceIncrement(ins["MinPriceIncrement"].toDouble());
    _model->setInstrumentID(ins["ZMInstrumentID"].toString());
    QJsonObject msg;
    QJsonObject header;
    header["MsgType"] = "V";  // MarketDataRequest
    header["ZMEndpoint"] = ins["ZMEndpoint"];
    msg["Header"] = header;
    QJsonObject body;
    body["ZMInstrumentID"] = ins["ZMInstrumentID"];
    body["SubscriptionRequestType"] = "1";  // snapshots and updates
    body["MDReqGrp"] = "*";  // all MDEntryTypes
    body["MDReqID"] = "domtrader";
    msg["Body"] = body;

    G::dealer->send(msg, marketDataReqResponse);
}

void Subscriber::unsubscribe(QJsonObject ins)
{
    if (ins.isEmpty())
        return;

    qDebug() << "unsubscribing from" << ins["Name"] << "...";

    auto marketDataReqResponse = [=](QJsonObject msg) {
        if (msg["Header"].toObject()["MsgType"].toString() == "ZMM") {
            // ZMMarketDataRequestResponse
            qDebug() << "unsubscribed" << ins["Name"];
        }
        qDebug() << msg;
    };

    QJsonObject msg;
    QJsonObject header;
    header["MsgType"] = "V";  // MarketDataRequest
    header["ZMEndpoint"] = ins["ZMEndpoint"];
    msg["Header"] = header;
    QJsonObject body;
    body["ZMInstrumentID"] = ins["ZMInstrumentID"];
    body["SubscriptionRequestType"] = "2"; // unsubscribe
    body["MDReqGrp"] = "*"; // all MDEntryType
    body["MDReqID"] = "domtrader";
    msg["Body"] = body;
    G::dealer->send(msg, marketDataReqResponse);

}


