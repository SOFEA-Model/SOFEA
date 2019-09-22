#ifndef SOURCETABLE_H
#define SOURCETABLE_H

#include "Scenario.h"
#include "SourceGroup.h"
#include "SourceModel.h"
#include "FluxProfileModel.h"
#include "widgets/StandardTableView.h"

#include <QAction>
#include <QColor>
#include <QLabel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QVector>

class SourceTable : public QWidget
{
    Q_OBJECT

public:
    SourceTable(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);
    double getTotalMass() const;

signals:
    void dataChanged();

public slots:
    void refresh();

private slots:
    void headerContextMenuRequested(const QPoint &pos);
    void contextMenuRequested(const QPoint &pos);
    void handleDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&);
    void handleRowsInserted(const QModelIndex&, int, int);
    void handleRowsRemoved(const QModelIndex&, int, int);

private:
    QColor colorFromIndex(const QModelIndex& index) const;
    void openColorEditor(const QModelIndexList& selection);
    void plotFluxProfile(const Source *s);
    void setColumnVisible(int column, bool visible);
    bool isColumnVisible(int column) const;

    Scenario *sPtr;
    SourceGroup *sgPtr;

    QLabel *massLabel;
    StandardTableView *table;
    SourceModel *model;
    QSortFilterProxyModel *proxyModel;
    FluxProfileModel *fpEditorModel;

    // Table Actions
    QAction *actAddArea;
    QAction *actAddAreaCirc;
    QAction *actAddAreaPoly;
    QAction *actImport;
    QAction *actEdit;
    QAction *actColor;
    QAction *actFlux;
    QAction *actRemove;
    QAction *actResampleAppStart;
    QAction *actResampleAppRate;
    QAction *actResampleIncorpDepth;
    QAction *actResampleFluxProfile;
};

#endif // SOURCETABLE_H
