#include <QWidget>
#include <QIcon>

#include "ScenarioProperties.h"
#include "Scenario.h"

ScenarioProperties::ScenarioProperties(Scenario *s, QWidget *parent)
    : SettingsDialog(parent), sPtr(s), saved(false)
{
    setWindowTitle(QString::fromStdString(s->title));
    setWindowIcon(QIcon(":/images/Settings_32x.png"));

    generalPage = new GeneralPage(s);
    metDataPage = new MetDataPage(s);
    dispersionPage = new DispersionPage(s);

    addPage("General Settings", generalPage);
    addPage("Meteorological Data", metDataPage);
    addPage("Dispersion Model", dispersionPage);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ScenarioProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ScenarioProperties::reject);
}

void ScenarioProperties::accept()
{
    generalPage->save();
    metDataPage->save();
    dispersionPage->save();

    saved = true;
}

void ScenarioProperties::reject()
{
    if (saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}
