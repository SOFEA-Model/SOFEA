#include <QPushButton>
#include <QWidget>
#include <QIcon>

#include "SourceGroupProperties.h"

SourceGroupProperties::SourceGroupProperties(Scenario *s, SourceGroup *sg, QWidget *parent)
    : SettingsDialog(parent), sgPtr(sg)
{
    setWindowTitle(QString::fromStdString(sg->grpid));
    setWindowIcon(QIcon(":/images/BuildQueue_32x.png"));

    applicationPage = new ApplicationPage(sg);
    depositionPage = new DepositionPage(sg);
    fluxProfilePage = new FluxProfilePage(s, sg);
    bufferZonePage = new BufferZonePage(sg);

    // Data from parent scenario.
    if (!s->aermodDryDeposition && !s->aermodWetDeposition) {
        depositionPage->warnDepoNotEnabled();
    }
    if (s->aermodGDVelocityEnabled) {
        depositionPage->warnUserVelocity();
    }

    addPage("Application", applicationPage);
    addPage("Deposition", depositionPage);
    addPage("Flux Profiles", fluxProfilePage);
    addPage("Buffer Zones", bufferZonePage);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SourceGroupProperties::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SourceGroupProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SourceGroupProperties::reject);
}

void SourceGroupProperties::apply()
{
    applicationPage->save();
    depositionPage->save();
    fluxProfilePage->save();
    bufferZonePage->save();

    // Recalculate source values if distribution is modified and in prospective mode.
    // TODO: remove isModified when random seed support is added. Add a menu option to explicitly resample selected parameters.
    if (!sgPtr->validationMode)
    {
        if (applicationPage->mcAppStart->isModified())
            sgPtr->resampleAppStart();
        if (applicationPage->mcAppRate->isModified())
            sgPtr->resampleAppRate();
        if (applicationPage->mcIncorpDepth->isModified())
            sgPtr->resampleIncorpDepth();
    }

    // Recalculate deposition values if modified, regardless of mode.
    if (depositionPage->mcAirDiffusion->isModified())
        sgPtr->resampleAirDiffusion();
    if (depositionPage->mcWaterDiffusion->isModified())
        sgPtr->resampleWaterDiffusion();
    if (depositionPage->mcCuticularResistance->isModified())
        sgPtr->resampleCuticularResistance();
    if (depositionPage->mcHenryConstant->isModified())
        sgPtr->resampleHenryConstant();

    emit saved();
}

void SourceGroupProperties::accept()
{
    apply();
    QDialog::done(QDialog::Accepted);
}

void SourceGroupProperties::reject()
{
    QDialog::done(QDialog::Rejected);
}
