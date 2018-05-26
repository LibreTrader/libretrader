#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QTcpServer>

#include "exceptions.h"


qint64 launchManagedChildProcessDetached(QProcess &childProcess,
                                         bool kill) {

    qint64 cpid;
    if (!childProcess.startDetached(&cpid)) {
        QString s = QString("failed to start child process: ");
        s += childProcess.program();
        throw RuntimeError(s);
    }
    // qDebug() << "managed child process started with pid" << cpid;

    qint64 ppid = QCoreApplication::applicationPid();
    QStringList args;
    args << QString::number(ppid) << QString::number(cpid);
    if (kill)
        args << "9";

    QProcess pcleaner;
    pcleaner.setProgram("./pcleaner");
    pcleaner.setArguments(args);
    pcleaner.setStandardOutputFile(QProcess::nullDevice());
    pcleaner.setStandardErrorFile(QProcess::nullDevice());

    if (!pcleaner.startDetached()) {
        qCritical() << "failed to start pcleaner with args:" << args;
        return cpid;
    }

    // qDebug() << "started pcleaner, args:" << args;
    return cpid;
}

void launchManagedChildProcess(QProcess &childProcess, bool kill)
{
    auto childStarted = [&]() {
        QProcess pcleaner;
        pcleaner.setProgram("./pcleaner");
        int ppid = QCoreApplication::applicationPid();
        int cpid = childProcess.processId();
        QStringList args;
        args << QString::number(ppid) << QString::number(cpid);
        if (kill)
            args << "9";
        pcleaner.setArguments(args);
        pcleaner.setStandardOutputFile(QProcess::nullDevice());
        pcleaner.setStandardErrorFile(QProcess::nullDevice());
        pcleaner.startDetached();
    };
    QObject::connect(&childProcess, &QProcess::started, childStarted);

    childProcess.start();
}

QJsonDocument parseToJsonDoc(const QByteArray &json)
{
    QJsonParseError err;
    QJsonDocument res = QJsonDocument::fromJson(json, &err);
    if (res.isNull())
    {
        qDebug() << err.offset << err.error;
        throw JsonParseError(err);
    }
    return res;
}

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
        res += QString::fromStdString(ba.toStdString()).replace("/", "\\/");
        res += "/";
    }
    return res.mid(0, res.size() - 1);
}

int countNumDecimals(double x) {
    int count = 0;
    x = x - (int)x;
    while (std::abs(x) > 1e-12) {
        x *= 10;
        x = x - (int)x;
        count++;
    }
    return count;
}

QString getLockFileName(QString path)
{
    QFileInfo fi(path);
    return fi.path() + QDir::separator() + ".#" + fi.fileName();
}

QList<quint16> getFreePorts(int n, quint16 minPort, quint16 maxPort)
{
    QList<quint16> res;
    QTcpServer server;
    for (quint16 port = minPort; port <= maxPort; port++) {
        if (server.listen(QHostAddress::LocalHost, port)) {
            // successfully bound to the port
            res.append(port);
            server.close();
        }
        if (res.size() >= n)
            break;
    }
    return res;
}


