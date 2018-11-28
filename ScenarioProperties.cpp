#include <QWidget>
#include <QIcon>

#include "ScenarioProperties.h"
#include "Scenario.h"

ScenarioProperties::ScenarioProperties(Scenario *s, QWidget *parent)
    : SettingsDialog(parent), sPtr(s), _saved(false)
{
    setWindowTitle(QString::fromStdString(s->title));
    setWindowIcon(QIcon(":/images/Settings_32x.png"));

    generalPage = new GeneralPage(s);
    metDataPage = new MetDataPage(s);
    fluxProfilesPage = new FluxProfilesPage(s);
    dispersionPage = new DispersionPage(s);

    addPage("General Settings", generalPage);
    addPage("Meteorological Data", metDataPage);
    addPage("Flux Profiles", fluxProfilesPage);
    addPage("Dispersion Model", dispersionPage);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ScenarioProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ScenarioProperties::reject);
}

void ScenarioProperties::accept()
{
    generalPage->save();
    metDataPage->save();
    fluxProfilesPage->save();
    dispersionPage->save();

    _saved = true;
    emit saved();
}

void ScenarioProperties::reject()
{
    if (_saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}
