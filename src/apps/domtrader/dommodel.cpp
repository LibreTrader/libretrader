#include "dommodel.h"

#include <algorithm>

#include <QSet>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "utils.h"

DomModel::DomModel(QObject *parent) : QAbstractListModel(parent)
{
    _roles[BidSizeRole] = "bidSize";
    _roles[AskSizeRole] = "askSize";
    _roles[PriceRole] = "price";
    _roles[ImpliedBidSizeRole] = "impliedBidSize";
    _roles[ImpliedAskSizeRole] = "impliedAskSize";
    _roles[HasBid] = "hasBid";
    _roles[HasAsk] = "hasAsk";
}

int DomModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _data.size();
}

QVariant DomModel::data(const QModelIndex &index, int role) const
{
    // qDebug() << index << role;

    int row = index.row();
    if (row < 0 || row >= _data.size())
        return QVariant();

    QSharedPointer<DomLevel> d = _data[row];

    if (role == PriceRole)
        return QString::number(d->price, 'f', _numDecimals);
    if (role == BidSizeRole)
        return d->bidSize;
    if (role == AskSizeRole)
        return d->askSize;
    if (role == ImpliedBidSizeRole)
        return d->impliedBidSize;
    if (role == ImpliedAskSizeRole)
        return d->impliedAskSize;
    if (role == HasBid)
        return d->bidSize > 0 || d->impliedBidSize > 0;
    if (role == HasAsk)
        return d->askSize > 0 || d->impliedAskSize > 0;


    return QVariant();
}

QSet<QString> BOOK_ENTRYTYPES = {
    "0", "1", "E", "F"
};

void DomModel::insertLevel(int index, QJsonObject data, bool emitSignals)
{
    QString et = data["MDEntryType"].toString();

    double price = data["MDEntryPx"].toDouble();
    int intPrice = std::round(price / _minPriceIncrement);
    double size = data["MDEntrySize"].toDouble();

    if (emitSignals)
        emit beginInsertRows(QModelIndex(), index, index);

    QSharedPointer<DomLevel> newLevel(new DomLevel(price, intPrice));
    if (et == "0") {  // bid
        newLevel->bidSize = size;
    } else if (et == "1") {  // ask
        newLevel->askSize = size;
    } else if (et == "E") {  // implied bid
        newLevel->impliedBidSize = size;
    } else if (et == "F") {  // implied ask
        newLevel->impliedAskSize = size;
    }

    _data.insert(index, newLevel);

    if (emitSignals)
        emit endInsertRows();
}

void DomModel::updateBook(QJsonObject data, QString updateAction,
                          bool emitSignals)
{
    // qDebug() << "updateBook: " << data;

    QString et = data["MDEntryType"].toString();

    double price = data["MDEntryPx"].toDouble();
    int intPrice = std::round(price / _minPriceIncrement);
    double size = data["MDEntrySize"].toDouble();

    for (int i = _data.size() - 1; i > -1; i--) {

        QSharedPointer<DomLevel> oldLevel = _data[i];

        if (intPrice == oldLevel->intPrice) {
            if (updateAction == "2") {  // delete
                if (et == "0") {  // bid
                    oldLevel->bidSize = 0;
                } else if (et == "1") {  // ask
                    oldLevel->askSize = 0;
                } else if (et == "E") {  // implied bid
                    oldLevel->impliedBidSize = 0;
                } else if (et == "F") {  // implied ask
                    oldLevel->impliedAskSize = 0;
                }
                if (oldLevel->isEmpty()) {
                    if (emitSignals)
                        emit beginRemoveRows(QModelIndex(), i, i);
                    _data.removeAt(i);
                    if (emitSignals)
                        emit endRemoveRows();
                } else {
                    if (emitSignals) {
                        QModelIndex idx = index(i);
                        emit dataChanged(idx, idx);
                    }
                }
            } else {  // insert or update
                if (et == "0") {  // bid
                    oldLevel->bidSize = size;
                } else if (et == "1") {  // ask
                    oldLevel->askSize = size;
                } else if (et == "E") {  // implied bid
                    oldLevel->impliedBidSize = size;
                } else if (et == "F") {  // implied ask
                    oldLevel->impliedAskSize = size;
                }
                if (emitSignals) {
                    QModelIndex idx = index(i);
                    emit dataChanged(idx, idx);
                }
            }
            //qDebug() << "UPD/REM " << et << " " << *oldLevel;
            return;
        }

        if (intPrice < oldLevel->intPrice) {
            if (updateAction == "2") {  // delete
                qWarning() << "invalid remove: " << endl << data;
                return;
            }
            insertLevel(i + 1, data, emitSignals);
            // qDebug() << "INS " << et << " " << *oldLevel;
            return;
        }
    }

    if (updateAction == "2") {
        qWarning() << "invalid remove: " << endl << data;
        return;
    }

    insertLevel(0, data, emitSignals);

}



void DomModel::update(QJsonObject data,
                      QString updateAction,
                      bool emitSignals)
{
    QString et = data["MDEntryType"].toString();

    if (BOOK_ENTRYTYPES.contains(et)) {
        updateBook(data, updateAction, emitSignals);
        return;
    }

//        QSharedPointer<DomLevel> lvl = new DomLevel();
//        lvl->isBid = true;
//        lvl->price = data["MDEntryPx"];
//        lvl->size = data["MDEntrySize"];
//        if (et == "0")
//            //        if (ua == "0") {
//            //            _bids.
//            //        }
//    }

}



void DomModel::updateIncremental(QJsonObject msg)
{
    QJsonObject body = msg["Body"].toObject();
    QJsonArray updates = body["MDIncGrp"].toArray();
    for (QJsonValue x : updates) {
        QString insID = x["ZMInstrumentID"].toString();
        if (insID != _instrumentID) {
            qDebug() << "skipped: " << insID;
            return;
        }
        update(x.toObject(), x["MDUpdateAction"].toString(), true);
    }
//    for (auto &x : _data) {
//        int bidsize = x->bidSize + x->impliedBidSize;
//        int asksize = x->askSize + x->impliedAskSize;
//        if (bidsize > 0 && asksize > 0) {

//        }
//    }
}


void DomModel::updateSnapshot(QJsonObject msg)
{
    QJsonObject body = msg["Body"].toObject();
    QString insID = body["ZMInstrumentID"].toString();
    if (insID != _instrumentID) {
        qDebug() << "skipped: " << insID;
        return;
    }
    emit beginResetModel();
    _clear();
    QJsonArray updates = body["MDFullGrp"].toArray();
    qDebug() << updates.size() << "updates on snapshot";
    for (QJsonValue x : updates) {
        update(x.toObject(), "0", false);
    }
    emit endResetModel();

    emit snapshotReceived();
}

void DomModel::clear()
{
    emit beginResetModel();
    _clear();
    emit endResetModel();
}

void DomModel::setMinPriceIncrement(double value)
{
    _minPriceIncrement = value;
    _numDecimals = countNumDecimals(value);
}

void DomModel::setInstrumentID(QString value)
{
    // qDebug() << "set insid to:" << value;
    _instrumentID = value;
}

int DomModel::centerIndex()
{
    int bidIdx = _data.size();
    int askIdx = 0;
    for (int i = 0; i < _data.size(); i++) {
        QSharedPointer<DomLevel> lvl = _data[i];
        if (lvl->bidSize > 0 || lvl->impliedBidSize > 0) {
            bidIdx = std::min(bidIdx, i);
        }
        if (lvl->askSize > 0 || lvl->impliedAskSize) {
            askIdx = std::max(askIdx, i);
        }
    }
    return std::round((bidIdx + askIdx) / 2.0);
}

void DomModel::_clear()
{
    _data.clear();
}

QDebug operator<<(QDebug dbg, const DomLevel &d)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace()
        << "{IP=" << d.intPrice << ", P=" << d.price
        << ", B=" << d.bidSize << ", IB=" << d.impliedBidSize
        << ", A=" << d.askSize << ", IA=" << d.impliedAskSize << "}";
    return dbg;
}
