#pragma once

#include <QAbstractButton>
#include <QSize>
#include <QString>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QWidget;
QT_END_NAMESPACE

class NavButton : public QAbstractButton
{
public:
    NavButton(const QString& text, QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual QSize sizeHint() const override;
};
