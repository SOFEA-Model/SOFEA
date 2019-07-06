#ifndef FLUXPROFILEPLOT_H
#define FLUXPROFILEPLOT_H

#include <QWidget>

#include "FluxProfile.h"
#include "StandardPlot.h"

QT_BEGIN_NAMESPACE
class QDateTime;
class QDateTimeEdit;
class QDoubleSpinBox;
QT_END_NAMESPACE

class FluxProfilePlot : public QWidget
{
    Q_OBJECT

public:
    FluxProfilePlot(FluxProfile fp, QWidget *parent = nullptr);
    void setupConnections();
    void setAppStart(const QDateTime& appStart);
    void setAppRate(double appRate);
    void setIncorpDepth(double incorpDepth);
    void setControlsEnabled(bool enabled);

public slots:
    void updatePlot();

private:
    QDateTimeEdit *deAppStart;
    QDoubleSpinBox *sbAppRate;
    QDoubleSpinBox *sbIncorpDepth;
    StandardPlot *plot;
    QwtPlotCurve *curve;
    FluxProfile fluxProfile;
};

#endif // FLUXPROFILEPLOT_H
