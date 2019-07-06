#include <QPushButton>
#include <QWidget>
#include <QIcon>

#include "ScenarioProperties.h"
#include "Scenario.h"

ScenarioProperties::ScenarioProperties(Scenario *s, QWidget *parent)
    : SettingsDialog(parent), sPtr(s)
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

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScenarioProperties::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ScenarioProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ScenarioProperties::reject);
}

void ScenarioProperties::apply()
{
    generalPage->save();
    metDataPage->save();
    fluxProfilesPage->save();
    dispersionPage->save();
    emit saved();
}

void ScenarioProperties::accept()
{
    apply();
    QDialog::done(QDialog::Accepted);
}

void ScenarioProperties::reject()
{
    QDialog::done(QDialog::Rejected);
}
