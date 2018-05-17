#ifndef SCENARIOPAGES_H
#define SCENARIOPAGES_H

#include <QWidget>

#include "Scenario.h"
#include "StandardTableView.h"
#include "Utilities.h"

QT_BEGIN_NAMESPACE
class QAction;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSpinBox;
QT_END_NAMESPACE

/****************************************************************************
** General
****************************************************************************/

class GeneralPage : public QWidget
{
    Q_OBJECT
public:
    GeneralPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *scenario;

    QComboBox *cboFumigant;
    DoubleLineEdit *leDecayCoefficient;
};

/****************************************************************************
** Meteorological Data
****************************************************************************/

class MetDataPage : public QWidget
{
    Q_OBJECT
public:
    MetDataPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void update();
    void browseMetDataFile();

private:
    Scenario *scenario;

    QLineEdit *leSurfaceDataFile;        // SURFFILE
    QPushButton *btnSurfaceDataFile;
    QLineEdit *leUpperAirDataFile;       // PROFFILE
    QPushButton *btnUpperAirDataFile;
    QDoubleSpinBox *sbAnemometerHeight;  // PROFBASE/ANEMHGHT
    QDoubleSpinBox *sbWindRotation;      // WDROTATE

    ReadOnlyLineEdit *leSurfaceStationId;
    ReadOnlyLineEdit *leUpperAirStationId;
    ReadOnlyLineEdit *leTotalHours;
    ReadOnlyLineEdit *leCalmHours;
    ReadOnlyLineEdit *leMissingHours;
    QListWidget *lwIntervals;
    QPushButton *btnUpdate;
};

/****************************************************************************
** Dispersion Model
****************************************************************************/

class DispersionPage : public QWidget
{
    Q_OBJECT
public:
    DispersionPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *scenario;

    // AERMOD
    QCheckBox *chkFlat;
    QCheckBox *chkFastArea;
    QCheckBox *chkLowWind;
    QWidget *lowWindParams;
    QDoubleSpinBox *sbSVmin;
    QDoubleSpinBox *sbWSmin;
    QDoubleSpinBox *sbFRANmax;

    // ISCST3
    QGroupBox *gbIscNonDefault;
    QCheckBox *chkIscNoCalm;
    QCheckBox *chkIscMsgPro;
};

#endif // SCENARIOPAGES_H
