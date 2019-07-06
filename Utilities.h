#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

#include <QDateEdit>
#include <QDockWidget>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPlainTextEdit>
#include <QProxyStyle>
#include <QStringList>
#include <QTextEdit>
#include <QToolButton>

//-----------------------------------------------------------------------------
// DockWidget
//-----------------------------------------------------------------------------

class DockWidgetProxyStyle : public QProxyStyle
{
public:
    DockWidgetProxyStyle(QStyle *style = nullptr);
protected:
    int pixelMetric(PixelMetric which, const QStyleOption *option,
        const QWidget *widget) const override;
    QIcon standardIcon(QStyle::StandardPixmap icon, const QStyleOption *option = nullptr,
        const QWidget *widget = nullptr) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget = nullptr) const override;
};

class DockWidget : public QDockWidget
{
public:
    DockWidget(const QString& title, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
    DockWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
private:
    DockWidgetProxyStyle *proxyStyle;
};

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
};

//-----------------------------------------------------------------------------
// ReadOnlyTextEdit
//-----------------------------------------------------------------------------

class ReadOnlyTextEdit : public QTextEdit
{
public:
    explicit ReadOnlyTextEdit(QWidget *parent = nullptr);
    void setLineCount(const int lines);
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
