#ifndef G_H
#define G_H

#include <QString>
#include "nzmqt/nzmqt.hpp"

class G
{
public:

    static QString brokerCtlAddr;
    static QString brokerPubAddr;
    static QString ctlAddr;
    static QString pubAddr;

    static nzmqt::ZMQContext *zctx;

    static QString configDir;
    static QString runDir;

    static QString pidFile;

    static void initialize();

private:
    explicit G() {}

};

#endif // G_H
