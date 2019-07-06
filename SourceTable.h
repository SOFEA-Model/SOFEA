#ifndef SOURCETABLE_H
#define SOURCETABLE_H

#include "Scenario.h"
#include "SourceGroup.h"
#include "SourceModel.h"
#include "FluxProfileModel.h"
#include "StandardTableView.h"
#include "Utilities.h"

#include <QAction>
#include <QLabel>
#include <QModelIndex>
#include <QVector>

class SourceTable : public QWidget
{
    Q_OBJECT

public:
    SourceTable(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);
    double getTotalMass() const;
    void plotFluxProfile(const Source *s);

signals:
    void dataChanged();

public slots:
    void refresh();

private slots:
    void headerContextMenuRequested(const QPoint &pos);
    void contextMenuRequested(const QPoint &pos);
    void handleDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);
    void handleRowsInserted(const QModelIndex &, int, int);
    void handleRowsRemoved(const QModelIndex &, int, int);

private:
    void setColumnVisible(int column, bool visible);
    bool isColumnVisible(int column) const;

    Scenario *sPtr;
    SourceGroup *sgPtr;

    QLabel *massLabel;
    StandardTableView *table;
    SourceModel *model;
    FluxProfileModel *fpEditorModel;

    // Table Actions
    QAction *actAddArea;
    QAction *actAddAreaCirc;
    QAction *actAddAreaPoly;
    QAction *actImport;
    QAction *actEdit;
    QAction *actFlux;
    QAction *actRemove;
    QAction *actResampleAppStart;
    QAction *actResampleAppRate;
    QAction *actResampleIncorpDepth;
    QAction *actResampleFluxProfile;
};

#endif // SOURCETABLE_H
