#ifndef RETURNINGDEALER_H
#define RETURNINGDEALER_H

#include <QList>
#include <QByteArray>
#include <QJSValue>
#include <QQmlEngine>

#include "nzmqt/nzmqt.hpp"

#include <functional>

typedef std::function<void(QJsonObject)> CPPCallbackFun;

class ReturningDealer : public QObject
{
    Q_OBJECT
public:
    explicit ReturningDealer(nzmqt::ZMQSocket *sock,
                             QQmlEngine *qmlEngine,
                             QObject *parent = nullptr);

    Q_INVOKABLE void send(QJSValue msg, QJSValue callback = QJSValue());
    void send(QJsonObject msg, const CPPCallbackFun& callback = nullptr);

private slots:
    void messageReceived(const QList<QByteArray> &msgParts);

private:
    nzmqt::ZMQSocket *_sock;
    QQmlEngine *_qmlEngine;
};

#endif // RETURNINGDEALER_H
