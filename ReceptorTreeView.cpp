#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QStylePainter>
#include <QDebug>

#include "ReceptorDelegate.h"
#include "ReceptorTreeView.h"

ReceptorTreeView::ReceptorTreeView(ReceptorModel *model, QWidget *parent)
    : QTreeView(parent)
{
    using Column = ReceptorModel::Column;

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);
    setPalette(QApplication::palette());
    setAutoFillBackground(true);
    setFrameStyle(QFrame::NoFrame);

    QFont font = QApplication::font();
    QHeaderView *header = this->header();
    header->setFont(font);
    header->setFixedHeight(32);

    setModel(model);

    setItemDelegateForColumn(Column::Color, new ReceptorColorDelegate);
    setItemDelegateForColumn(Column::Group, new ReceptorGroupDelegate);
    setItemDelegateForColumn(Column::Type, new ReceptorGroupDelegate);
    setItemDelegateForColumn(Column::X, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::Y, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::Z, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::ZHill, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::ZFlag, new ReceptorGridDelegate);

    const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    QFontMetrics fm(font);
    const int charWidth = fm.averageCharWidth();

    setColumnWidth(Column::Color, indentation() + iconSize * 1.5);
    setColumnWidth(Column::Group, charWidth * 9);
    setColumnWidth(Column::Type, charWidth * 8);
    setColumnWidth(Column::X, charWidth * 8);
    setColumnWidth(Column::Y, charWidth * 8);
    setColumnWidth(Column::Z, charWidth * 7);
    setColumnWidth(Column::ZHill, charWidth * 7);
    setColumnWidth(Column::ZFlag, charWidth * 7);
}

ReceptorTreeView::~ReceptorTreeView()
{}

void ReceptorTreeView::selectLastRow()
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    if (nrows > 0) {
        QItemSelectionModel *sm = selectionModel();
        QModelIndex index = model()->index(nrows - 1, 0, QModelIndex());
        sm->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        scrollToBottom();
    }
}

void ReceptorTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = viewOptions();

    // Fill the branch rect.
    QRect bgrect = QRect(rect.topLeft(), QPoint(rect.left() + indentation() - 1, rect.bottom()));
    painter->fillRect(bgrect, opt.palette.window());

    QTreeView::drawBranches(painter, rect, index);
}

void ReceptorTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;

    // Always use active style for selections.
    if (selectionModel()->isSelected(index))
        opt.state |= QStyle::State_Active;

    QTreeView::drawRow(painter, opt, index);

    // Draw horizontal gridline below the row.
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(color);
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
    painter->restore();
}


