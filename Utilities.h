#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPlainTextEdit>
#include <QStringList>
#include <QTextEdit>
#include <QToolButton>

//-----------------------------------------------------------------------------
// GridLayout
//-----------------------------------------------------------------------------

class GridLayout : public QGridLayout
{
public:
    GridLayout();
};

//-----------------------------------------------------------------------------
// BackgroundFrame
//-----------------------------------------------------------------------------

class BackgroundFrame : public QFrame
{
public:
    BackgroundFrame();
};

//-----------------------------------------------------------------------------
// PlainTextEdit
//-----------------------------------------------------------------------------

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit PlainTextEdit(QWidget *parent = nullptr);
public slots:
    void appendPlainText(const std::string &text);
    void insertPlainText(const std::string &text);
    void setPlainText(const std::string &text);
};

//-----------------------------------------------------------------------------
// DoubleLineEdit
//-----------------------------------------------------------------------------

class DoubleLineEdit : public QLineEdit
{
public:
    explicit DoubleLineEdit(QWidget *parent = nullptr);
    DoubleLineEdit(double min, double max, int decimals, QWidget *parent = nullptr);
    double value();
    void setValue(double value);
private:
    double m_min;
    double m_max;
    int m_decimals;
};

//-----------------------------------------------------------------------------
// ReadOnlyLineEdit
//-----------------------------------------------------------------------------

class ReadOnlyLineEdit : public QLineEdit
{
public:
    explicit ReadOnlyLineEdit(QWidget *parent = nullptr);
    void setBasePalette();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPalette m_defaultPalette;
};

//-----------------------------------------------------------------------------
// ReadOnlyTextEdit
//-----------------------------------------------------------------------------

class ReadOnlyTextEdit : public QTextEdit
{
public:
    explicit ReadOnlyTextEdit(QWidget *parent = nullptr);
    void setRowCount(const int rows);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPalette m_defaultPalette;
};

//-----------------------------------------------------------------------------
// StatusLabel
//-----------------------------------------------------------------------------

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

#endif // UTILITIES_H
