#include "returningdealer.h"

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QThread>

QHash<QByteArray, QJSValue> callbacksJS;
QHash<QByteArray, CPPCallbackFun> callbacksCPP;

quint64 seqNo = 0;

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
    QByteArray msgId = QString::number(seqNo++).toUtf8();
    callbacksJS[msgId] = callback;
    QByteArray msgBytes = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    QList<QByteArray> msgParts = { "",  msgId, " " + msgBytes };
    _sock->sendMessage(msgParts);
}

void ReturningDealer::send(QJsonObject msg, const CPPCallbackFun &callback)
{
    QByteArray msgId = QString::number(seqNo++).toUtf8();
    if (callback)
        callbacksCPP[msgId] = callback;
    QByteArray msgBytes = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    QList<QByteArray> msgParts = { "",  msgId, " " + msgBytes };
    _sock->sendMessage(msgParts);
}

void ReturningDealer::messageReceived(const QList<QByteArray> &msgParts)
{
    QJsonObject msg = QJsonDocument::fromJson(
                msgParts[msgParts.size() - 1]).object();
    QByteArray msgId = msgParts[msgParts.size() - 2];

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
