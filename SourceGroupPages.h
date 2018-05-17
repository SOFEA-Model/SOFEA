#ifndef SOURCEGROUPPAGES_H
#define SOURCEGROUPPAGES_H

#include <QWidget>

#include "MonteCarloLineEdit.h"
#include "MonteCarloDateTimeEdit.h"
#include "StandardTableView.h"
#include "StandardTableEditor.h"
#include "Utilities.h"

#include "SourceGroup.h"
#include "SourceModel.h"
#include "SourceEditor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDataWidgetMapper;
class QDateEdit;
class QDoubleSpinBox;
class QGroupBox;
class QItemSelection;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class QStandardItemModel;
QT_END_NAMESPACE

/****************************************************************************
** Application
****************************************************************************/

class ApplicationPage : public QWidget
{
    Q_OBJECT
public:
    ApplicationPage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

    MonteCarloDateTimeEdit *mcAppStart;
    MonteCarloLineEdit *mcAppRate;
    MonteCarloLineEdit *mcIncorpDepth;

private:
    SourceGroup *sgPtr;

    static const QMap<SourceGroup::AppMethod, QString> appMethodMap;

    QComboBox *cboAppMethod;
    QDoubleSpinBox *sbAppFactor;
    QButtonGroup *bgCalcMode;
    QRadioButton *radioProspective;
    QRadioButton *radioValidation;
    QGroupBox *gbMonteCarlo;
};

/****************************************************************************
** Flux Profile
****************************************************************************/

class FluxProfilePage : public QWidget
{
    Q_OBJECT
public:
    FluxProfilePage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    SourceGroup *sgPtr;

    static const QMap<FluxScaling::TSMethod, QString> tsMethodMap;
    static const QMap<FluxScaling::DSMethod, QString> dsMethodMap;

    // Reference
    QDateTimeEdit *deRefDate;
    QDoubleSpinBox *sbRefAppRate;
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

private slots:
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void importFluxProfile();
    void calcPhase();
    void plotTemporalScaling();
    void plotDepthScaling();
};

/****************************************************************************
** Buffer Zone
****************************************************************************/

class BufferZonePage : public QWidget
{
    Q_OBJECT
public:
    BufferZonePage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void onAddZoneClicked();
    void onRemoveZoneClicked();

private:
    SourceGroup *sgPtr;

    std::vector<std::pair<double, int>> zones;

    QDoubleSpinBox *sbBuffer;
    QSpinBox *sbReentry;
    QStandardItemModel *zoneModel;
    StandardTableView *zoneTable;
    StandardTableEditor *zoneEditor;
};

/****************************************************************************
** Fields
****************************************************************************/

class FieldPage : public QWidget
{
    Q_OBJECT
public:
    FieldPage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    SourceGroup *sgPtr;
    SourceModel *sourceModel;

    static const QMap<SourceType, QString> sourceTypeMap;

    QPushButton *btnAddSource;
    QPushButton *btnRemoveSource;
    QPushButton *btnImport;
    QMenu *btnAddSourceMenu;
    QAction *actAddArea;
    QAction *actAddAreaCirc;
    QAction *actAddAreaPoly;
    StandardTableView *sourceTable;
    SourceEditor *sourceEditor;

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void removeSelectedRows();
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onRowsRemoved(const QModelIndex& parent, int first, int last);
    void import();
};

#endif // SOURCEGROUPPAGES_H
