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

#pragma once

#include <memory>
#include <vector>

#include <QBrush>
#include <QDialog>
#include <QPointF>

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QDateTimeEdit;
class QDialog;
class QDialogButtonBox;
class QDoubleSpinBox;
class QListView;
class QPainter;
class QPointF;
class QRadioButton;
class QSpinBox;
class QStandardItemModel;
QT_END_NAMESPACE

class ctkRangeSlider;
class QwtPolarGrid;
class ReadOnlyLineEdit;

#include <qwt_polar_curve.h>
#include <qwt_polar_plot.h>
#include <qwt_scale_map.h>

#include "core/Meteorology.h"

//-----------------------------------------------------------------------------
// WindRoseSector
//-----------------------------------------------------------------------------

class WindRoseSector : public QwtPolarCurve
{
public:
    void setBrush(const QBrush &);
    const QBrush &brush() const;

private:
    void drawCurve(QPainter *, int style,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QPointF &pole, int from, int to) const override;

    void drawSector(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QPointF &pole, int from, int to) const;

    QBrush d_brush;
};

//-----------------------------------------------------------------------------
// WindRosePlot
//-----------------------------------------------------------------------------

class WindRosePlot : public QwtPolarPlot
{
public:
    explicit WindRosePlot(QWidget *parent = nullptr);

private:
    QwtPolarGrid *d_grid;
};

//-----------------------------------------------------------------------------
// MeteorologyInfoDialog
//-----------------------------------------------------------------------------

class MeteorologyInfoDialog : public QDialog
{
    Q_OBJECT

public:
    MeteorologyInfoDialog(const Meteorology& m, QWidget *parent = nullptr);

private:
    void init();
    void drawSectors();

private slots:
    void onSliderChanged(const int min, const int max);
    void onDateTimeChanged(const QDateTime& datetime);
    void onSectorSizeChanged(int id);
    void onBinCountChanged(int value);

private:
    std::vector<SurfaceRecord> surfaceData;

    // Data Controls
    ctkRangeSlider *timeRangeSlider;
    QDateTimeEdit *dteMinTime;
    QDateTimeEdit *dteMaxTime;
    ReadOnlyLineEdit *leTotalHours;
    ReadOnlyLineEdit *leCalmHours;
    ReadOnlyLineEdit *leMissingHours;

    // Plot Controls
    QButtonGroup *bgSectorSize;
    QRadioButton *rbSectorSize10;
    QRadioButton *rbSectorSize15;
    QRadioButton *rbSectorSize30;
    QSpinBox *sbBinCount;

    QStandardItemModel *binModel;
    QListView *binView;
    QDialogButtonBox *buttonBox;

    WindRosePlot *wrPlot;
    std::vector<WindRoseSector *> wrSectors;
    std::vector<QColor> wrColors;

    // Wind Rose Parameters
    double azimuthSpacing = 1.5;
    int idxMin = 0;
    int idxMax = 0;
    int wdBinCount = 24;
    int wsBinCount = 5;
    double wsMin = 0;
    double wsMax = 10;
};
