#pragma once

#include <QPlainTextEdit>

#include <string>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class PlainTextEdit : public QPlainTextEdit
{
public:
    explicit PlainTextEdit(QWidget *parent = nullptr);

    void setLineCount(int lines);
    void appendPlainText(const std::string &text);
    void insertPlainText(const std::string &text);
    void setPlainText(const std::string &text);
};
