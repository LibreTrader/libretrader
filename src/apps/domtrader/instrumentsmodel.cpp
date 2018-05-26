#include "instrumentsmodel.h"

#include <algorithm>

#include <QLockFile>
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QJsonArray>
#include <QDebug>

#include "g.h"
#include "utils.h"

InstrumentsModel::InstrumentsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    _roles[ModelData] = "modelData";
}

int InstrumentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _data.size();
}

QVariant InstrumentsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= _data.size())
        return QVariant();

    QJsonValue d = _data[row];

    if (role == ModelData)
        return d["Name"];

    return QVariant();
}

QJSValue InstrumentsModel::jsObject(int index) const
{
    return G::qmlEng->toScriptValue(_data[index]);
}

QJsonObject InstrumentsModel::at(int index) const
{
    return _data[index];
}

int InstrumentsModel::indexOf(QString name) const
{
    for (int i = 0; i < _data.size(); i++) {
        if (_data[i]["Name"] == name)
            return i;
    }
    return -1;
}

int InstrumentsModel::indexOf(QJSValue item) const
{
    if (!item.hasProperty("Name"))
        return -1;
    return indexOf(item.property("Name").toString());
}

void InstrumentsModel::loadFromFile(QString filename)
{
    qDebug() << "loading instruments from:" << filename << "...";

    QLockFile lockFile(getLockFileName(filename));
    lockFile.lock();

    if (!QFileInfo(filename).exists()) {
        qInfo() << filename << "not found";
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "failed to open file:" << filename;
        exit(1);
    }
    QByteArray data = file.readAll();

    loadFromBytes(data);
}

void InstrumentsModel::loadFromBytes(QByteArray data)
{
    QJsonDocument jdoc;
    try {
        jdoc = parseToJsonDoc(data);
    } catch (const Exception& err) {
        qCritical() << err;
        exit(1);
    }

    emit beginResetModel();

    _data.clear();
    for (QJsonValue x : jdoc.array()) {
        _data.append(x.toObject());
    }

    emit endResetModel();

    qDebug() << _data.size() << "entries loaded";
}
