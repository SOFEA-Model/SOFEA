#ifndef SOURCETABLE_H
#define SOURCETABLE_H

#include "SourceModel.h"
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
    SourceTable(SourceGroup *sg, QWidget *parent = nullptr);
    double getTotalMass() const;
    void showFluxProfile(const Source *s);

signals:
    void dataChanged();

public slots:
    void refresh();

private slots:
    void showContextMenu(const QPoint &pos);
    void handleDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);
    void handleRowsInserted(const QModelIndex &, int, int);
    void handleRowsRemoved(const QModelIndex &, int, int);

private:
    SourceGroup *sgPtr;
    QLabel *massLabel;
    StandardTableView *table;
    SourceModel *model;
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
