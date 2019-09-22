#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QPainter>
#include <QStylePainter>
#include <QProxyStyle>
#include <QRect>

class AppStyle : public QProxyStyle
{
    Q_OBJECT

public:
    AppStyle();
protected:
    int pixelMetric(PixelMetric which, const QStyleOption *option,
        const QWidget *widget) const override;
    QIcon standardIcon(QStyle::StandardPixmap icon, const QStyleOption *option = nullptr,
        const QWidget *widget = nullptr) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
        const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option,
        QStyle::SubControl sc, const QWidget *widget) const override;
};

#endif // APPSTYLE_H
