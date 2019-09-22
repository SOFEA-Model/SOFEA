#ifndef SOURCEGROUPPAGES_H
#define SOURCEGROUPPAGES_H

#include <QWidget>

#include "BufferZoneModel.h"
#include "FluxProfileModel.h"
#include "SamplingDistributionEditor.h"
#include "MonteCarloLineEdit.h"
#include "MonteCarloDateTimeEdit.h"
#include "widgets/StandardTableEditor.h"
#include "widgets/StandardTableView.h"
#include "widgets/StatusLabel.h"

#include "Scenario.h"
#include "SourceGroup.h"

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
** Deposition
****************************************************************************/

class DepositionPage : public QWidget
{
public:
    DepositionPage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void warnDepoNotEnabled();
    void warnUserVelocity();
    void load();
    void save();

    MonteCarloLineEdit *mcAirDiffusion;
    MonteCarloLineEdit *mcWaterDiffusion;
    MonteCarloLineEdit *mcCuticularResistance;
    MonteCarloLineEdit *mcHenryConstant;

private:
    StatusLabel *lblDepoNotEnabled;
    StatusLabel *lblDepoUserVelocity;
    SourceGroup *sgPtr;
};

/****************************************************************************
** Flux Profile
****************************************************************************/

class FluxProfilePage : public QWidget
{
    Q_OBJECT
public:
    FluxProfilePage(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *sPtr;
    SourceGroup *sgPtr;

    FluxProfileModel *model;
    SamplingDistributionEditor *editor;
    StatusLabel *lblNoFluxProfile;
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
    void importBufferZoneTable(const QString& filename);

private:
    SourceGroup *sgPtr;

    QCheckBox *chkEnable;
    QDoubleSpinBox *sbAreaThreshold;
    QDoubleSpinBox *sbAppRateThreshold;
    BufferZoneModel *zoneModel;
    StandardTableView *zoneTable;
    StandardTableEditor *zoneEditor;
    StatusLabel *lblThresholdInfo;
};

#endif // SOURCEGROUPPAGES_H
