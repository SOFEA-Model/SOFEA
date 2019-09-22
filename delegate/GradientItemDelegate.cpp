#include "GradientItemDelegate.h"

#include <QImage>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>

GradientItemDelegate::GradientItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void GradientItemDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::DecorationRole).canConvert<QLinearGradient>()) {
        QLinearGradient gradient = qvariant_cast<QLinearGradient>(index.data(Qt::DecorationRole));
        QPixmap pixmap = gradientPixmap(gradient);

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        const int x = option.rect.x();
        const int y = option.rect.y();

        painter->drawPixmap(QRect(x, y, pixmap.rect().width(), pixmap.rect().height()), pixmap);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize GradientItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(48, 16);
}

QPixmap GradientItemDelegate::gradientPixmap(const QLinearGradient &gradient)
{
    QImage img(48, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), gradient);
    painter.end();

    return QPixmap::fromImage(img);
}
