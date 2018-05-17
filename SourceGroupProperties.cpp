#include <QWidget>
#include <QIcon>

#include "SourceGroupProperties.h"

SourceGroupProperties::SourceGroupProperties(SourceGroup *sg, QWidget *parent)
    : SettingsDialog(parent), sgPtr(sg), saved(false)
{
    setWindowTitle(QString::fromStdString(sg->grpid));
    setWindowIcon(QIcon(":/images/BuildQueue_32x.png"));

    applicationPage = new ApplicationPage(sg);
    fluxProfilePage = new FluxProfilePage(sg);
    bufferZonePage = new BufferZonePage(sg);
    fieldPage = new FieldPage(sg);

    addPage("Application", applicationPage);
    addPage("Flux Profile", fluxProfilePage);
    addPage("Buffer Zone", bufferZonePage);
    addPage("Fields", fieldPage);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SourceGroupProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SourceGroupProperties::reject);
}

void SourceGroupProperties::accept()
{   
    applicationPage->save();
    fluxProfilePage->save();
    bufferZonePage->save();
    fieldPage->save();

    // Recalculate source values if distribution is modified and in prospective mode.
    if (!sgPtr->validationMode)
    {
        if (applicationPage->mcAppStart->isModified())
            sgPtr->resampleAppStart();
        if (applicationPage->mcAppRate->isModified())
            sgPtr->resampleAppRate();
        if (applicationPage->mcIncorpDepth->isModified())
            sgPtr->resampleIncorpDepth();
    }

    saved = true;
}

void SourceGroupProperties::reject()
{
    if (saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}
