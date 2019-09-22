#include "PointDelegate.h"

#include <QMetaType>
#include <QPainter>
#include <QPointF>

PointDelegate::PointDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QString PointDelegate::displayText(const QVariant& value, const QLocale&) const
{
    if (value.type() == QMetaType::QPoint) {
        QPoint p = value.toPoint();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else if (value.type() == QMetaType::QPointF) {
        QPointF p = value.toPointF();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else {
        return QString();
    }
}

void PointDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    // Workaround for QPoint(0,0) which returns isNull() == true
    if ((index.data(Qt::DisplayRole).type() == QVariant::Point) ||
        (index.data(Qt::DisplayRole).type() == QVariant::PointF))
    {
        painter->drawText(option.rect,
                          displayText(index.data(), QLocale::system()),
                          option.displayAlignment);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
