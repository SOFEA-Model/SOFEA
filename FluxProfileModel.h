#ifndef FLUXPROFILEMODEL_H
#define FLUXPROFILEMODEL_H

#include <memory>
#include <vector>

#include <QAbstractTableModel>
#include <QMap>
#include <QVariant>

#include "FluxProfile.h"

class FluxProfileModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    FluxProfileModel(QObject *parent = nullptr);

    void save(std::vector<std::shared_ptr<FluxProfile>>& profiles) const;
    void load(const std::vector<std::shared_ptr<FluxProfile> > &profiles, bool copy = true);
    void reset();
    std::shared_ptr<FluxProfile> fluxProfileFromIndex(const QModelIndex &index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceFirst, int count,
                  const QModelIndex &destinationParent, int destinationFirst) override;

private:
    std::vector<std::shared_ptr<FluxProfile>> modelData;
};

#endif // FLUXPROFILEMODEL_H
