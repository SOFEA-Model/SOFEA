#pragma once

#include <QFrame>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class GridLineFrame : public QFrame
{
public:
    explicit GridLineFrame(Qt::Orientation orientation, QWidget *parent = nullptr);
};
