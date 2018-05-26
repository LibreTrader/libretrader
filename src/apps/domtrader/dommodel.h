#ifndef DOMMODEL_H
#define DOMMODEL_H

#include <QHash>
#include <QList>
#include <QAbstractListModel>
#include <QSet>
#include <QJsonObject>
#include <QSharedPointer>

class DomLevel {

public:

    explicit DomLevel(double price, int intPrice) {
        this->price = price;
        this->intPrice = intPrice;
    }

    bool isEmpty() {
        if (bidSize != 0) return false;
        if (askSize != 0) return false;
        if (impliedBidSize != 0) return false;
        if (impliedAskSize != 0) return false;
        return true;
    }

    double bidSize = 0;
    double askSize = 0;
    double impliedBidSize = 0;
    double impliedAskSize = 0;
    double price;
    int intPrice;

};

QDebug operator<<(QDebug dbg, const DomLevel &d);


class DomModel : public QAbstractListModel
{

    Q_OBJECT

public:

    enum Roles {
        BidSizeRole = Qt::UserRole + 1,
        AskSizeRole,
        ImpliedBidSizeRole,
        ImpliedAskSizeRole,
        PriceRole,
        HasBid,
        HasAsk
    };

    explicit DomModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override { return _roles; }
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void updateIncremental(QJsonObject msg);
    void updateSnapshot(QJsonObject msg);
    Q_INVOKABLE void clear();

    void setMinPriceIncrement(double value);
    void setInstrumentID(QString value);

    Q_INVOKABLE int centerIndex();

signals:

    void snapshotReceived();


private:

    void _clear();
    void update(QJsonObject data, QString updateAction, bool emitSignals);
    void updateBook(QJsonObject data, QString updateAction, bool emitSignals);
    void insertLevel(int index, QJsonObject data, bool emitSignals);

    QHash<int, QByteArray> _roles;
    QList<QSharedPointer<DomLevel>> _data;
    double _tickSize;
    int _numDecimals;
    double _minPriceIncrement;
    QString _instrumentID;

};

#endif // DOMMODEL_H
