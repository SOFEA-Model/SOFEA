#pragma once

#include <QFrame>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QWidget;
QT_END_NAMESPACE

class BackgroundFrame : public QFrame
{
public:
    explicit BackgroundFrame(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};
