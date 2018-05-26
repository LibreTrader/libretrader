#include "applauncher.h"

#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QRegularExpression>

#include "g.h"
#include "utils.h"

typedef void (QProcess::*procFinishSigType)(int, QProcess::ExitStatus);

AppLauncher::AppLauncher(QObject *parent) : QObject(parent)
{
}

AppLauncher::~AppLauncher()
{
    // Assumption: all child processes will properly manage their subprocesses.
    // With this assumption it is safe to kill them upon Launcher close.
    // Otherwise they may leave zombie child processes behind.

    // Need a shallow copy because the original container is mutated during
    // the iteration.
    QSet<QProcess*> copy(_processes);

    // finished signal will still fire and be handled normally ...
    for (QProcess *process : copy) {
        _processes.remove(process);
        process->kill();
    }

    for (QProcess *process : copy) {
        process->waitForFinished();
        delete process;
    }
}

void AppLauncher::launchDomTrader()
{
    qDebug() << "launching domtrader ...";

    QString insFile = G::configDir + QDir::separator() + "instruments.json";
    QProcess *process = new QProcess;
    process->setProgram("./domtrader");
    QStringList args = {
        insFile,
        G::ctlAddr,
        G::pubAddr,
        "--broker-pub",
        QString(G::brokerPubAddr).replace(QRegularExpression("tcp://.*:"),
                                          "tcp://localhost:"),
    };
    qDebug() << args;
    process->setArguments(args);

    launchProcess(process);
}

void AppLauncher::launchInstrumentSelector()
{
    qDebug() << "launching instrument-selector ...";

    QString insFile = G::configDir + QDir::separator() + "instruments.json";
    QProcess *process = new QProcess;
    process->setProgram("./instrument-selector");
    QStringList args = {
        insFile,
        G::ctlAddr,
        "--broker-ctl",
        QString(G::brokerCtlAddr).replace(QRegularExpression("tcp://.*:"),
                                          "tcp://localhost:"),
    };
    qDebug() << args;
    process->setArguments(args);
    launchProcess(process);
}

int AppLauncher::processCount()
{
    return _processes.size();
}

void AppLauncher::launchProcess(QProcess *process)
{
    _processes.insert(process);
    auto started = [=]() {
        process->setObjectName(QString::number(process->processId()));
        qDebug().nospace()
                << "process (" << process->objectName().toInt()
                << ") started: " << process->program();
    };
    connect(process, &QProcess::started, started);

    auto finished = [=](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug().nospace()
                << "process (" << process->objectName().toInt()
                << ") exited: code=" << exitCode
                << ", status=" << exitStatus;
        if (_processes.contains(process)) {
            _processes.remove(process);
            delete process;
        }
    };
    connect(process,
            // tell compiler which overload of QProcess::finished to use
            static_cast<procFinishSigType>(&QProcess::finished),
            finished);

    launchManagedChildProcess(*process);
}
