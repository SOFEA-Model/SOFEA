#ifndef SOURCEGROUPPROPERTIES_H
#define SOURCEGROUPPROPERTIES_H

#include "SettingsDialog.h"
#include "SourceGroupPages.h"

class SourceGroupProperties : public SettingsDialog
{
    Q_OBJECT

public:
    SourceGroupProperties(SourceGroup *sg, QWidget *parent = nullptr);

public slots:
    void accept() override;
    void reject() override;

private:
    SourceGroup *sgPtr;
    ApplicationPage *applicationPage;
    FluxProfilePage *fluxProfilePage;
    BufferZonePage *bufferZonePage;
    FieldPage *fieldPage;
    bool saved;
};

#endif // SOURCEGROUPPROPERTIES_H
