#ifndef PIXMAPUTILITIES_H
#define PIXMAPUTILITIES_H

#include <QBrush>
#include <QLinearGradient>
#include <QPixmap>

class PixmapUtilities
{
public:
    static QPixmap brushValuePixmap(const QBrush &brush);
    static QPixmap gradientPixmap(const QLinearGradient &gradient);
};

#endif // PIXMAPUTILITIES_H
