#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include <QObject>
#include <QSet>
#include <QProcess>

class AppLauncher : public QObject
{
    Q_OBJECT

public:

    explicit AppLauncher(QObject *parent = nullptr);
    ~AppLauncher();

    Q_INVOKABLE void launchDomTrader();
    Q_INVOKABLE void launchInstrumentSelector();
    Q_INVOKABLE int processCount();

private:

    void launchProcess(QProcess* process);

    QSet<QProcess*> _processes;

};

#endif // APPLAUNCHER_H
