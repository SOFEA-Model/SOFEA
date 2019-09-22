#pragma once

#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class ReadOnlyLineEdit : public QLineEdit
{
public:
    explicit ReadOnlyLineEdit(QWidget *parent = nullptr);
    void setBasePalette();
};
