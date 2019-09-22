#ifndef FLUXPROFILEDIALOG_H
#define FLUXPROFILEDIALOG_H

#include <memory>

#include <QDialog>
#include <QMap>

#include "FluxProfile.h"
#include "widgets/StandardTableEditor.h"
#include "widgets/StandardTableView.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QComboBox;
class QDateEdit;
class QDateTimeEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class QStandardItemModel;
QT_END_NAMESPACE

class FluxProfileDialog : public QDialog
{
    Q_OBJECT

public:
    FluxProfileDialog(std::shared_ptr<FluxProfile> fp, QWidget *parent = nullptr);

private:
    void init();
    void save();
    void load();
    FluxProfile currentProfile();

private slots:
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void importFluxProfile(const QString& filename);
    void calcPhase();
    void plotFluxProfile();
    void plotTemporalScaling();
    void plotDepthScaling();

public slots:
    void accept() override;
    
private:
    std::weak_ptr<FluxProfile> wptr;

    static const QMap<FluxProfile::TSMethod, QString> tsMethodMap;
    static const QMap<FluxProfile::DSMethod, QString> dsMethodMap;

    QDialogButtonBox *buttonBox;
    QPushButton *btnPlot;

    // Reference
    QDateTimeEdit *deRefDate;
    QDoubleSpinBox *sbRefAppRate;
    //QButtonGroup *bgFluxMode;
    //StatusLabel *lblConstantFluxInfo;
    //QRadioButton *radioConstantFlux;
    //QRadioButton *radioVariableFlux;
    QDoubleSpinBox *sbRefDepth;
    QDoubleSpinBox *sbRefVL;
    QDoubleSpinBox *sbMaxVL;
    QStandardItemModel *refModel;
    StandardTableView *refTable;
    StandardTableEditor *refEditor;

    // Temporal Scaling
    QComboBox *cboTemporalScaling;
    QStackedWidget *temporalStack;

    // Temporal Scaling - CDPR
    QDateEdit *deStartDate;
    QDateEdit *deEndDate;
    QDoubleSpinBox *sbScaleFactor;

    // Temporal Scaling - Sinusoidal
    QDoubleSpinBox *sbAmplitude;
    QDoubleSpinBox *sbCenterAmplitude;
    QDoubleSpinBox *sbPhase;
    QDoubleSpinBox *sbWavelength;
    QPushButton *btnCalcPhase;
    QPushButton *btnPlotTS;

    // Incorporation Depth Scaling
    QComboBox *cboDepthScaling;
    QPushButton *btnPlotDS;
};

#endif // FLUXPROFILEDIALOG_H
