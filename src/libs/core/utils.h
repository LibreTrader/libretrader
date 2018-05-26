#ifndef UTILS_H
#define UTILS_H

#include <functional>

#include <QList>

#include "exceptions.h"

class QProcess;
class QJsonDocument;
class QJsonArray;
class QJsonObject;
class QByteArray;
class QString;

qint64 launchManagedChildProcessDetached(QProcess &childProcess,
                                         bool kill = false);

void launchManagedChildProcess(QProcess &childProcess,
                               bool kill = false);

QJsonDocument parseToJsonDoc(const QByteArray &json);

int indexOfSeparator(const QList<QByteArray> &msgParts);

QString identToString(const QList<QByteArray> &ident);

int countNumDecimals(double x);

QString getLockFileName(QString path);

QList<quint16> getFreePorts(int n, quint16 minPort=1025,
                            quint16 maxPort=65535);

#endif // UTILS_H
