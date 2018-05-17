#ifndef UTILITIES_H
#define UTILITIES_H

#include <QDateEdit>
#include <QGridLayout>
#include <QFrame>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPalette>

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
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPalette currentPalette;
};

#endif // UTILITIES_H
