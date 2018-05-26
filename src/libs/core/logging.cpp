#include "logging.h"

#include <QTextStream>
#include <QFile>
#include <QDateTime>

QTextStream _stdout(stdout);
QTextStream _stderr(stderr);

void basicConsoleHandler(QtMsgType type,
                         const QMessageLogContext &context,
                         const QString &msg) {
#ifndef QT_DEBUG
    // skip debug messages on release builds
    if (type == QtDebugMsg) return;
#endif
    Q_UNUSED(context);
    QTextStream *stream;
    const char *severity;
    switch (type) {
        case QtDebugMsg:
            stream = &_stdout;
            severity = "[DEBUG]";
            break;
        case QtInfoMsg:
            stream = &_stdout;
            severity = "[INFO]";
            break;
        case QtWarningMsg:
            stream = &_stderr;
            severity = "[WARNING]";
            break;
        case QtCriticalMsg:
            stream = &_stderr;
            severity = "[CRITICAL]";
            break;
        case QtFatalMsg:
            stream = &_stderr;
            severity = "[FATAL]";
            break;
        default:
            stream = &_stdout;
            severity = "[UNKNOWN]";
    }
    QDateTime dt = QDateTime::currentDateTimeUtc();
    QString dtStr = dt.toString("hh:mm:ss.zzz");
    (*stream) << dtStr << " " << severity << " " << msg << endl;
}


void setupBasicConsoleLogging()
{
    qInstallMessageHandler(basicConsoleHandler);
}
