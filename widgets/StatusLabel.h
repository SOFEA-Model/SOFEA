#pragma once

#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class StatusLabel : public QWidget
{
public:
    StatusLabel(QWidget *parent = nullptr);
    void setSeverity(const int severity);
    void setText(const QString& text);

private:
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
};
