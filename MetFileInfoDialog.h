#ifndef METFILEINFODIALOG_H
#define METFILEINFODIALOG_H

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

#include "MetFileParser.h"

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
    void onSliderChanged(const int min, const int max);
    void onDateTimeChanged(const QDateTime& datetime);
    void onSectorSizeChanged(int id);
    void onBinCountChanged(int value);

private:
    std::shared_ptr<SurfaceData> sd;

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

#endif // METFILEINFODIALOG_H
