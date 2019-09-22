#ifndef SOURCEMODEL_H
#define SOURCEMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QMap>

#include "Scenario.h"
#include "SourceGroup.h"
#include "Projection.h"

class SourceModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ID,
        X,
        Y,
        Z,
        Longitude,
        Latitude,
        Area,
        Start,
        AppRate,
        IncDepth,
        MassAF,
        FluxProfile,
        TimeSF,
        DepthSF,
        OverallSF,
        BZDistance,
        BZDuration,
        AirDiffusion,
        WaterDiffusion,
        CuticularResistance,
        HenryConstant
    };

    SourceModel(Scenario *s, SourceGroup *sg, QObject *parent = nullptr);
    void reset();
    void import();
    void setProjection(const Projection::Generic& p);
    void setColumnHidden(int column, bool hidden);
    bool isColumnHidden(int column) const;
    Source* sourceFromIndex(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;

public slots:
    void addAreaSource();
    void addAreaCircSource();
    void addAreaPolySource();

private:
    Scenario *sPtr;
    SourceGroup *sgPtr;
    QMap<int, bool> columnHidden;
    Projection::Transform transform;
};

#endif // SOURCEMODEL_H
