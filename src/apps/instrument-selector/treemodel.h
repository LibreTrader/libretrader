#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QVariant>
#include <QHash>
#include <QJSValue>
#include <QJsonArray>

class TreeItem
{

public:

    TreeItem(const QList<QVariant> &data);
    ~TreeItem();

    void appendChild(TreeItem *child);
    void populate(QJsonArray data);

    void clear();

    bool populated() { return _populated; }
    QString instrumentID() { return _instrumentID; }

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

private:

    QList<TreeItem*> _children;
    QList<QVariant> _data;
    TreeItem *_parent = nullptr;
    bool _populated = false;
    QString _instrumentID;

};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Roles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole
    };

    explicit TreeModel(QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override { return _roles; }

    Q_INVOKABLE void populateNode(QJSValue data,
                                  QModelIndex index = QModelIndex());
    Q_INVOKABLE bool isPopulated(QModelIndex index);
    Q_INVOKABLE QString getPath(QModelIndex index);
    Q_INVOKABLE QString getInstrumentID(QModelIndex index);

private:

    TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *_root;
    QHash<int, QByteArray> _roles;

};

#endif // TREEMODEL_H
