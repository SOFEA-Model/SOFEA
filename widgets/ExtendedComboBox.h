#pragma once

#include <QComboBox>
#include <QList>

QT_BEGIN_NAMESPACE
class QFrame;
class QWidget;
QT_END_NAMESPACE

class ExtendedComboBox : public QComboBox
{
public:
    explicit ExtendedComboBox(QWidget *parent = nullptr);
    void setPopupHeight(int height);
    int popupHeight() const;

protected:
    virtual void showPopup() override;

private:
    QFrame *container_ = nullptr;
    QList<QWidget *> scrollers_;
    int popupHeight_ = 0;
};
