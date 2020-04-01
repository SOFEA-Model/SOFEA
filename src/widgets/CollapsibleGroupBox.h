#pragma once

#include <QGroupBox>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidget>
QT_BEGIN_NAMESPACE
class QChildEvent;
class QMouseEvent;
class QPaintEvent;
QT_END_NAMESPACE

class CollapsibleGroupBox : public QGroupBox
{
public:
    explicit CollapsibleGroupBox(QWidget *parent = nullptr);
    CollapsibleGroupBox(const QString& title, QWidget *parent = nullptr);

    bool collapsed() const;
    void setCollapsed(bool);
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void childEvent(QChildEvent *) override;
    void setChildrenEnabled(bool);

private:
    QRect textRect();
    QRect collapsedRect();
    
    bool collapsed_;
    bool pressed_;
};
