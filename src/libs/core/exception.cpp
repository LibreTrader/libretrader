#include "exception.h"
#include <sstream>
#include <iosfwd>
#include <QDebug>
#include <QTextStream>

Exception::Exception(QVariant arg) noexcept {
    _whatArg = arg;
    backward::StackTrace st;
    st.load_here(100);
    // 3 first frames are about producing the stack trace itself, not relevant.
    st.skip_n_firsts(3);
    _traceback = st;
}

QString Exception::toString() const {
#ifdef Q_OS_LINUX
        int status;
        char *klass = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
        QString s = QString("%1: %2").arg(klass).arg(_whatArg.toString());
        free(klass);
        return s;
#else
        const char* klass = typeid(*this).name();
        return QString("%1: %2").arg(klass).arg(_whatArg.toString());
#endif
}

void Exception::printTrace(FILE *file) const {
    QTextStream stream(file);
    printTrace(stream);
}

void Exception::printTrace(QTextStream &stream) const {
    stream << formatTrace();
}

QString Exception::formatTrace() const {
    std::ostringstream stream;
    backward::Printer p;
    backward::StackTrace traceback = _traceback;
    p.print(traceback, stream);  // non-const reference to traceback
    return QString::fromStdString(stream.str());
}

QDebug operator<<(QDebug out, const Exception &exc) {
    //out << exc.toString().toUtf8().constData();
    out << qPrintable(exc.toString());
    return out;
}
