#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "exception.h"

struct QJsonParseError;

class JsonParseError : public Exception {
public:
    explicit JsonParseError(QJsonParseError err);
};

class ArgumentException : public Exception {
public:
    explicit ArgumentException(QVariant arg) : Exception(arg) {}
};

class RuntimeError : public Exception {
public:
    explicit RuntimeError(QVariant arg) : Exception(arg) {}
};

#endif // EXCEPTIONS_H
