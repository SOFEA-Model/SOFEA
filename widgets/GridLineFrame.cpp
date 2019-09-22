#include "GridLineFrame.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QWidget>

GridLineFrame::GridLineFrame(Qt::Orientation orientation, QWidget *parent)
    : QFrame(parent)
{
    setContentsMargins(0, 0, 0, 0);
    if (orientation == Qt::Horizontal)
        setFrameShape(QFrame::HLine);
    else
        setFrameShape(QFrame::VLine);
    setFrameShadow(QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(0);

    QStyleOptionViewItem opt;
    int hint = QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt);
    QColor color = static_cast<QRgb>(hint);

    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, color);
    setPalette(palette);
}
