// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
                              //  Index   FieldType  Editable   SourceType   ValueType
    enum Column {             //  -----   ---------  --------   ----------   --------------------
        ID,                   //      0   Basic      Yes        *            std::string
        X,                    //      1   Basic      Yes        *            double
        Y,                    //      2   Basic      Yes        *            double
        Z,                    //      3   Basic      Yes        *            double
        Longitude,            //      4   Basic      Yes        *            double
        Latitude,             //      5   Basic      Yes        *            double
        Area,                 //      6   Basic      No         *            double
        Start,                //      7   Basic      Yes        *            DateTimeDistribution
        AppRate,              //      8   Basic      Yes        *            GenericDistribution
        IncDepth,             //      9   Basic      Yes        *            GenericDistribution
        MassAF,               //     10   Basic      No         *            double
        FluxProfile,          //     11   Basic      Yes        *            SamplingDistribution
        TimeSF,               //     12   Basic      No         *            double
        DepthSF,              //     13   Basic      No         *            double
        OverallSF,            //     14   Basic      No         *            double
        BZDistance,           //     15   Basic      Yes        *            double
        BZDuration,           //     16   Basic      Yes        *            int
        AirDiffusion,         //     17   Basic      Yes        *            GenericDistribution
        WaterDiffusion,       //     18   Basic      Yes        *            GenericDistribution
        CuticularResistance,  //     19   Basic      Yes        *            GenericDistribution
        HenryConstant,        //     20   Basic      Yes        *            GenericDistribution
        XInit,                //     21   Extended   Yes        AREA         double
        YInit,                //     22   Extended   Yes        AREA         double
        Angle,                //     23   Extended   Yes        AREA         double
        Radius,               //     24   Extended   Yes        AREACIRC     double
        VertexCount,          //     25   Extended   Yes        AREACIRC     int
        EmissionRate,         //     26   Extended   Yes        *            double
        ReleaseHeight,        //     27   Extended   Yes        *            double
        SigmaZ0,              //     28   Extended   Yes        *            double
        VertexData            //     29   Extended   Yes        *            QPolygonF
    };

    SourceModel(Scenario *s, SourceGroup *sg, QObject *parent = nullptr);
    void reset();
    void import();
    void setProjection(const Projection::Generic& p);
    void setColumnHidden(int column, bool hidden);
    bool isColumnHidden(int column) const;
    Source* sourceFromIndex(const QModelIndex &index) const;
    QModelIndex vertexIndex(int row, int i) const;
    void emitDataChanged(const QModelIndex& index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
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
