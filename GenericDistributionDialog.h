#ifndef GENERICDISTRIBUTIONDIALOG_H
#define GENERICDISTRIBUTIONDIALOG_H

#include <QDialog>

#include "GenericDistribution.h"
#include "StandardPlot.h"

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QStandardItemModel;
class QTreeView;
QT_END_NAMESPACE

enum class DistributionID
{
    UniformInt,
    UniformReal,
    Binomial,
    Geometric,
    NegativeBinomial,
    Poisson,
    Exponential,
    Gamma,
    Weibull,
    ExtremeValue,
    Beta,
    Laplace,
    Normal,
    Lognormal,
    ChiSquared,
    NCChiSquared,
    Cauchy,
    FisherF,
    StudentT,
    Discrete,
    PiecewiseConstant,
    PiecewiseLinear,
    Triangle
};

class GenericDistributionDialog : public QDialog
{
    Q_OBJECT

public:
    GenericDistributionDialog(const GenericDistribution &d, QWidget *parent = nullptr);
    GenericDistribution getDistribution() const;

private:
    void initializeModel();
    void initializeConnections();
    void resetControls();
    void setDistribution(DistributionID id);
    void updatePlot();

private slots:
    void onValueChanged(double);

private:
    GenericDistribution currentDist;
    DistributionID currentID;

    QStandardItemModel *distributionModel;
    QTreeView *distributionTree;

    QLabel *label1;
    QLabel *label2;
    QLabel *label3;

    QDoubleSpinBox *sb1;
    QDoubleSpinBox *sb2;
    QDoubleSpinBox *sb3;

    StandardPlot *pdfPlot;
    QwtPointSeriesData *pdfPlotCurveData;
    QwtPlotCurve *pdfPlotCurve;

    QDialogButtonBox *buttonBox;
};

#endif // GENERICDISTRIBUTIONDIALOG_H
