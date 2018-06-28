#ifndef METFILEINFODIALOG_H
#define METFILEINFODIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QRadioButton>
#include <QSpinBox>
#include <QBrush>
#include <QPainter>
#include <QPointF>

#include <ctkRangeSlider.h>
#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>

#include "MetFileParser.h"
#include "Utilities.h"

class QwtPolarGrid;

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

    void drawPolygon(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QPointF &pole, int from, int to) const;

    QBrush d_brush;
};

//-----------------------------------------------------------------------------
// WindRosePlot
//-----------------------------------------------------------------------------

class WindRosePlot : public QwtPolarPlot
{
    Q_OBJECT
public:
    explicit WindRosePlot(QWidget *parent = nullptr);

private:
    QwtPolarGrid *d_grid;
};

//-----------------------------------------------------------------------------
// MetFileInfoDialog
//-----------------------------------------------------------------------------

class MetFileInfoDialog : public QDialog
{
    Q_OBJECT
public:
    MetFileInfoDialog(std::shared_ptr<SurfaceData> sd, QWidget *parent = nullptr);

private:
    void init();
    void drawSectors();

private slots:
    void onRangeChanged(const int min, const int max);

private:
    std::shared_ptr<SurfaceData> sd;
    std::vector<WindRoseSector *> sectors;

    // Data Controls
    ctkRangeSlider *timeRangeSlider;
    ReadOnlyLineEdit *leStartTime;
    ReadOnlyLineEdit *leEndTime;
    ReadOnlyLineEdit *leTotalHours;
    ReadOnlyLineEdit *leCalmHours;
    ReadOnlyLineEdit *leMissingHours;

    // Plot Controls
    QButtonGroup *bgSectorSize;
    QRadioButton *rbSectorSize10;
    QRadioButton *rbSectorSize15;
    QRadioButton *rbSectorSize30;
    QSpinBox *sbBinCount;

    // Plot Widgets
    WindRosePlot *wrPlot;

    // Wind Rose Parameters
    double azimuthSpacing = 1.5;
    int idxMin = 0;
    int idxMax = 0;
    int wdBinCount = 24;
    int wsBinCount = 5;
    double wsMin = 0;
    double wsMax = 10;
    std::vector<QColor> colors;
};

#endif // METFILEINFODIALOG_H
