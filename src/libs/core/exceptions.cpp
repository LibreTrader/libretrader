#include "exceptions.h"

#include <QJsonParseError>

JsonParseError::JsonParseError(QJsonParseError err)
{

    _whatArg = QString("Error parsing JSON on offset %1: %2")
            .arg(err.offset).arg(err.errorString());
}
