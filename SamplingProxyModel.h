#ifndef SAMPLINGPROXYMODEL_H
#define SAMPLINGPROXYMODEL_H

#include <map>

#include <QIdentityProxyModel>

class SamplingProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit SamplingProxyModel(QObject *parent = nullptr);

    int extraColumnForProxyColumn(int proxyColumn) const;
    int proxyColumnForExtraColumn(int extraColumn) const;

    // Overrides
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &i) const override;

public slots:
    void normalize();

private:
    bool normalizeInternal(std::map<QModelIndex, double>& map) const;

    std::map<QModelIndex, double> modelData;
};

#endif // SAMPLINGPROXYMODEL_H
