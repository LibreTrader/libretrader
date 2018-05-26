#ifndef ZMQUTILS_H
#define ZMQUTILS_H

#include <QList>
#include <QByteArray>
#include <QString>

int indexOfSeparator(const QList<QByteArray> &msgParts);

QString identToString(const QList<QByteArray> &ident);

#endif // ZMQUTILS_H
