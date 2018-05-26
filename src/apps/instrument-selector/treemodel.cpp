#include "treemodel.h"

#include <QDebug>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

TreeItem::TreeItem(const QList<QVariant> &data)
{
    _parent = nullptr;
    _data = data;
}

TreeItem::~TreeItem()
{
    qDeleteAll(_children);
}

void TreeItem::appendChild(TreeItem *child)
{
    qDebug() << "appending" << child << "to" << this << "...";
    child->_parent = this;
    _children.append(child);
}

void TreeItem::populate(QJsonArray data)
{
    for (auto x : data) {
        QJsonObject d = x.toObject();
        TreeItem *item = new TreeItem(
                { d["ZMNodeName"].toString(), d["Text"].toString() });
        if (!d.contains("ZMInstrumentID")) {
            item->appendChild(new TreeItem({ "...", ""}));
            // qDebug() << "children after dummy insert: " << item->childCount();
        } else {
            item->_instrumentID = d["ZMInstrumentID"].toString();
        }
        appendChild(item);
    }
    _populated = true;
}

void TreeItem::clear()
{
    // qDebug() << "clearing" << this << "...";
    for (TreeItem *child : _children) {
        //qDebug() << "deleting" << child << "...";
        delete child;
    }
    // qDeleteAll(_children);
    _children.clear();
}

TreeItem *TreeItem::child(int row)
{
    return _children.value(row);
}

int TreeItem::childCount() const
{
    return _children.count();
}

int TreeItem::columnCount() const
{
    return _data.count();
}

QVariant TreeItem::data(int column) const
{
    return _data.value(column);
}

TreeItem *TreeItem::parent()
{
    return _parent;
}

int TreeItem::row() const
{
    if (_parent)
        return _parent->_children.indexOf(const_cast<TreeItem*>(this));

    return 0;
}


TreeModel::TreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    _root = new TreeItem({ "Name", "Description" });

    _roles = QAbstractItemModel::roleNames();
    _roles[NameRole] = "name";
    _roles[DescriptionRole] = "description";
}

TreeModel::~TreeModel() {
    delete _root;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return getItem(parent)->columnCount();
    else
        return _root->columnCount();
}



QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem* node = getItem(index);

    switch (role) {
        case NameRole:
            return node->data(0);
        case DescriptionRole:
            return node->data(1);
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return _root->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == _root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

void TreeModel::populateNode(QJSValue data, QModelIndex index)
{
    QJsonArray arr = QJsonDocument::fromVariant(data.toVariant()).array();

    TreeItem *item = getItem(index);

    // qDebug() << "populating node" << item << "...";

    if (item->childCount() > 0) {
        // delete dummy item
        emit beginRemoveRows(index, 0, item->childCount() - 1);
        item->clear();
        emit endRemoveRows();
    }

    if (arr.empty())
        return;

    if (item == _root) {
        emit beginResetModel();
    } else {
        emit beginInsertRows(index, 0, arr.size() - 1);
    }

    item->populate(arr);

    if (item == _root) {
        emit endResetModel();
    } else {
        emit endInsertRows();
    }

//    emit beginResetModel();
//    emit endResetModel();
}

bool TreeModel::isPopulated(QModelIndex index)
{
    TreeItem *item;
    if (!index.isValid()) {
        item = _root;
    } else {
//        TreeItem* parent = static_cast<TreeItem*>(index.internalPointer());
//        item = parent->child(index.row());
        item = getItem(index);
    }
    return item->populated();
}

QString TreeModel::getPath(QModelIndex index)
{
    if (!index.isValid())
        return QString();

    TreeItem *item = getItem(index);

    QList<QString> res;
    while (item->parent()) {
        res.append(item->data(0).toString());
        item = item->parent();
    }
    std::reverse(res.begin(), res.end());

    return "/" + res.join("/");
}

QString TreeModel::getInstrumentID(QModelIndex index)
{
    if (!index.isValid())
        return QString();

    TreeItem *item = getItem(index);
    return item->instrumentID();
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return _root;
}

