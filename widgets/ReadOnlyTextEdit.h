#pragma once

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class ReadOnlyTextEdit : public QTextEdit
{
public:
    explicit ReadOnlyTextEdit(QWidget *parent = nullptr);
    void setLineCount(int lines);
};
