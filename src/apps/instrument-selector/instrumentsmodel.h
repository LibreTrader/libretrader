#ifndef INSTRUMENTSMODEL_H
#define INSTRUMENTSMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QVariant>
#include <QJsonObject>
#include <QJSValue>


class InstrumentsModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum Roles {
        ModelData = Qt::UserRole + 1
    };

    explicit InstrumentsModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override { return _roles; }
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE int update(QJSValue data);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE QJSValue jsObject(int index) const;
    QJsonObject at(int index) const;
    Q_INVOKABLE int indexOf(QString name) const;
    Q_INVOKABLE int indexOf(QJSValue item) const;
    Q_INVOKABLE void loadFromFile(QString filename);

private:

    void saveToFile() const;
    void broadcastData(QString data) const;

    QString _fileName;
    QHash<int, QByteArray> _roles;
    QVector<QJsonObject> _data;

};

#endif // INSTRUMENTSMODEL_H
