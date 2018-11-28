#ifndef SAMPLINGDISTRIBUTIONEDITOR_H
#define SAMPLINGDISTRIBUTIONEDITOR_H

#include <QWidget>
#include <QString>

#include "SamplingProxyModel.h"
#include "SamplingDistribution.h"
#include "StandardTableView.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QStandardItemModel;
QT_END_NAMESPACE

class SamplingDistributionEditor : public QWidget
{
    Q_OBJECT

public:
    SamplingDistributionEditor(QAbstractItemModel *model, QWidget *parent = nullptr);
    void setColumnHidden(int column);
    bool setProbability(int row, double value);
    double getProbability(int row) const;
    void normalize();

private slots:
    void onSetClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    QDoubleSpinBox *sbProbability;
    SamplingProxyModel *proxyModel;
    StandardTableView *view;
    QPushButton *btnSet;
    QPushButton *btnNormalize;
};

#endif // SAMPLINGDISTRIBUTIONEDITOR_H
