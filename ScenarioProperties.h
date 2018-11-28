#ifndef SCENARIOPROPERTIES_H
#define SCENARIOPROPERTIES_H

#include "Scenario.h"
#include "ScenarioPages.h"
#include "SettingsDialog.h"

class ScenarioProperties : public SettingsDialog
{
    Q_OBJECT

public:
    ScenarioProperties(Scenario *s, QWidget *parent = nullptr);

signals:
    void saved();

public slots:
    void accept() override;
    void reject() override;

private:
    Scenario *sPtr;
    GeneralPage *generalPage;
    MetDataPage *metDataPage;
    FluxProfilesPage *fluxProfilesPage;
    DispersionPage *dispersionPage;
    bool _saved;
};

#endif // SCENARIOPROPERTIES_H
