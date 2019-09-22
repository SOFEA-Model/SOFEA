#pragma once

#include <QGridLayout>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class GridLayout : public QGridLayout
{
public:
    explicit GridLayout(QWidget *parent = nullptr);
};
