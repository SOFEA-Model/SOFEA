#ifndef RECEPTORTREEVIEW_H
#define RECEPTORTREEVIEW_H

#include <QTreeView>
#include <QPainter>
#include <QPaintEvent>

#include "ReceptorModel.h"

class ReceptorTreeView : public QTreeView
{
    Q_OBJECT

public:
    ReceptorTreeView(ReceptorModel *model, QWidget *parent = nullptr);
    ~ReceptorTreeView();

    void selectLastRow();

protected:
    void drawBranches(QPainter *painter, const QRect& rect, const QModelIndex& index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // RECEPTORTREEVIEW_H
