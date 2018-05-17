#ifndef SCENARIOPROPERTIES_H
#define SCENARIOPROPERTIES_H

#include "SettingsDialog.h"
#include "ScenarioPages.h"

class ScenarioProperties : public SettingsDialog
{
    Q_OBJECT

public:
    ScenarioProperties(Scenario *s, QWidget *parent = nullptr);

public slots:
    void accept() override;
    void reject() override;

private:
    Scenario *sPtr;
    GeneralPage *generalPage;
    MetDataPage *metDataPage;
    DispersionPage *dispersionPage;
    bool saved;
};

#endif // SCENARIOPROPERTIES_H
