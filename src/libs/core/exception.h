#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>

#include "stdio.h"
#include <QVariant>
#include "backward.hpp"

class QTextStream;

class Exception : public std::exception
{

public:

    explicit Exception() noexcept : Exception(QVariant()) {}
    explicit Exception(QVariant arg) noexcept;
    QString formatTrace() const;
    void printTrace(FILE *file = stderr) const;
    void printTrace(QTextStream &stream) const;
    virtual QString toString() const;

    virtual const char* what() const noexcept {
        return _whatArg.toString().toUtf8().constData();
    }
    backward::StackTrace traceback() const { return _traceback; }

protected:

    QVariant _whatArg;
    backward::StackTrace _traceback;

    friend QDebug operator<<(QDebug out, const Exception &exc);
};

#endif // EXCEPTION_H
