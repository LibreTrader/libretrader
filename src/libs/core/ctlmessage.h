#ifndef CTLMESSAGE_H
#define CTLMESSAGE_H

#include <QList>
#include <QByteArray>
#include <QJsonObject>

class CTLMessage
{

public:

    explicit CTLMessage() {}
    explicit CTLMessage(const QList<QByteArray> &msgParts);

    QList<QByteArray> ident;
    QByteArray msgID;
    QJsonObject payload;

};

#endif // CTLMESSAGE_H
