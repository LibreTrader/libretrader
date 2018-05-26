#include "ctlmessage.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "utils.h"


CTLMessage::CTLMessage(const QList<QByteArray> &msgParts)
{
    int sepIdx = indexOfSeparator(msgParts);
    ident = msgParts.mid(0, sepIdx);
    if (msgParts.size() != sepIdx + 3)
        throw ArgumentException("invalid number of frames");
    msgID = msgParts[sepIdx + 1];
    payload = parseToJsonDoc(msgParts.last()).object();
}
