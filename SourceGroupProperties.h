#ifndef SOURCEGROUPPROPERTIES_H
#define SOURCEGROUPPROPERTIES_H

#include "Scenario.h"
#include "SourceGroup.h"
#include "SourceGroupPages.h"
#include "SettingsDialog.h"

class SourceGroupProperties : public SettingsDialog
{
    Q_OBJECT

public:
    SourceGroupProperties(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);

signals:
    void saved();

public slots:
    void apply();
    void accept() override;
    void reject() override;

private:
    SourceGroup *sgPtr;
    ApplicationPage *applicationPage;
    DepositionPage *depositionPage;
    FluxProfilePage *fluxProfilePage;
    BufferZonePage *bufferZonePage;
};

#endif // SOURCEGROUPPROPERTIES_H
