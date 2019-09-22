#pragma once

#include <QBrush>
#include <QPixmap>

class PixmapUtilities
{
public:
    static QPixmap brushValuePixmap(const QBrush &brush);
};
