#include <QWidget>
#include <QIcon>

#include "SourceGroupProperties.h"

SourceGroupProperties::SourceGroupProperties(Scenario *s, SourceGroup *sg, QWidget *parent)
    : SettingsDialog(parent), sgPtr(sg), _saved(false)
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
    addPage("Flux Profile", fluxProfilePage);
    addPage("Buffer Zone", bufferZonePage);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SourceGroupProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SourceGroupProperties::reject);
}

void SourceGroupProperties::accept()
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

    _saved = true;
    emit saved();
}

void SourceGroupProperties::reject()
{
    if (_saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}
