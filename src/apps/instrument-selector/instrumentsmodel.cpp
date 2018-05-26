#include "instrumentsmodel.h"

#include <QFileInfo>
#include <QFile>
#include <QLockFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QTextStream>

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

int InstrumentsModel::update(QJSValue data)
{
    QJsonObject obj = QJsonDocument::fromVariant(data.toVariant()).object();
    for (int i = 0; i < _data.size(); i++) {
        QJsonObject old = _data[i];
        if (old["Name"] == obj["Name"]) {
            _data[i] = obj;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx);
            saveToFile();
            return i;
        }
    }
    emit beginInsertRows(QModelIndex(), _data.size(), _data.size());
    _data.append(obj);
    emit endInsertRows();
    saveToFile();
    return _data.size() - 1;
}

void InstrumentsModel::remove(int index)
{
    emit beginRemoveRows(QModelIndex(), index, index);
    _data.remove(index);
    emit endRemoveRows();
    saveToFile();
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
    _fileName = filename;

    qDebug() << "loading instruments from:" << filename << "...";

    QLockFile lockFile(getLockFileName(filename));
    lockFile.lock();

    if (!QFileInfo(filename).exists()) {
        qDebug() << filename << "not found";
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "failed to open file:" << filename;
        exit(1);
    }
    QByteArray data = file.readAll();

    QJsonDocument jdoc;
    try {
        jdoc = parseToJsonDoc(data);
    } catch (const Exception& err) {
        qCritical() << err;
        exit(1);
    }

    for (QJsonValue x : jdoc.array()) {
        _data.append(x.toObject());
    }

    qDebug() << _data.size() << "entries loaded from file";
}

void InstrumentsModel::saveToFile() const
{
    qDebug() << "saving to" << _fileName << " ...";

    QLockFile lockFile(getLockFileName(_fileName));
    lockFile.lock();

    QFile file(_fileName);
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);

    QJsonArray arr;
    for (QJsonObject obj : _data) {
        arr.append(obj);
    }

    QByteArray insData = QJsonDocument(arr).toJson();

    stream << insData;

    file.close();
    lockFile.unlock();

    broadcastData(insData);
}

void InstrumentsModel::broadcastData(QString data) const
{
    if (!G::brokerDealer) {
        return;
    }
    qDebug() << "broadcasting instrument_list ...";
    QJsonObject msg;
    msg["msg_type"] = "broadcast";
    QJsonObject body;
    body["topic"] = "instruments_list";
    body["data"] = data;
    msg["body"] = body;
    G::brokerDealer->send(msg);
}
