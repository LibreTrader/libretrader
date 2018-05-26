#include "zmqutils.h"

#include "exceptions.h"

#include <QList>
#include <QByteArray>
#include <QDebug>

int indexOfSeparator(const QList<QByteArray> &msgParts)
{
    int separatorIdx = -1;
    for (int i = 0 ; i < msgParts.size(); i++) {
        QByteArray frame = msgParts[i];
        if (frame.isEmpty()) {
            separatorIdx = i;
            break;
        }
    }
    if (separatorIdx == -1) throw ArgumentException("separator not found");
    return separatorIdx;
}

// use QT_USE_STRINGBUILDER macro for perf boost
// http://doc.qt.io/qt-5/qstring.html#more-efficient-string-construction
QString identToString(const QList<QByteArray> &ident)
{
    QString res;
    for (const QByteArray &ba : ident) {
        res += ba.replace("/", "\\/") + "/";
    }
    return res.mid(0, res.size() - 1);
}
