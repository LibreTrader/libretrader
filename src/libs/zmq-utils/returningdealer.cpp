#include "returningdealer.h"

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

QHash<QString, QJSValue> callbacksJS;
QHash<QString, CPPCallbackFun> callbacksCPP;

qint64 seqNo = 0;

ReturningDealer::ReturningDealer(nzmqt::ZMQSocket *sock,
                                 QQmlEngine *qmlEngine,
                                 QObject *parent)
    : QObject(parent)
{
    _sock = sock;
    _qmlEngine = qmlEngine;
    QObject::connect(sock,
                     &nzmqt::ZMQSocket::messageReceived,
                     this,
                     &ReturningDealer::messageReceived);
}

void ReturningDealer::send(QJSValue data, QJSValue callback)
{
    QJsonObject msg = QJsonDocument::fromVariant(data.toVariant()).object();
    QString msgId = QString::number(seqNo++);
    msg["msg_id"] = msgId;
    callbacksJS[msgId] = callback;
    QByteArray msgBytes = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    QList<QByteArray> msgParts = { "",  " " + msgBytes };
    _sock->sendMessage(msgParts);
}

void ReturningDealer::send(QJsonObject msg, const CPPCallbackFun &callback)
{
    QString msgId = QString::number(seqNo++);
    msg["msg_id"] = msgId;
    if (callback)
        callbacksCPP[msgId] = callback;
    QByteArray msgBytes = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    QList<QByteArray> msgParts = { "",  " " + msgBytes };
    _sock->sendMessage(msgParts);
}

void ReturningDealer::messageReceived(const QList<QByteArray> &msgParts)
{
    //qDebug() << msgParts;
    QJsonObject msg = QJsonDocument::fromJson(
                msgParts[msgParts.size() - 1]).object();
    QString msgId = msg["msg_id"].toString();

    QJSValue callbackJS = callbacksJS.value(msgId, QJSValue());
    if (callbackJS.isCallable()) {
        QJSValue msgJS = _qmlEngine->toScriptValue(msg);
        callbackJS.call(QJSValueList { msgJS });
    }

    if (callbacksCPP.contains(msgId)) {
        CPPCallbackFun callbackCPP = callbacksCPP[msgId];
        callbackCPP(msg);
    }
}
