#include "PixmapUtilities.h"

#include <QColor>
#include <QImage>
#include <QPainter>

QPixmap PixmapUtilities::brushValuePixmap(const QBrush& brush)
{
    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), brush);
    QColor color = brush.color();
    if (color.alpha() != 255) { // indicate alpha by an inset
        QBrush opaqueBrush = brush;
        color.setAlpha(255);
        opaqueBrush.setColor(color);
        painter.fillRect(img.width() / 4, img.height() / 4,
                         img.width() / 2, img.height() / 2, opaqueBrush);
    }
    painter.end();

    return QPixmap::fromImage(img);
}

QPixmap PixmapUtilities::gradientPixmap(const QLinearGradient &gradient)
{
    QImage img(48, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), gradient);
    painter.end();

    return QPixmap::fromImage(img);
}
