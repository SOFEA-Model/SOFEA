#include "AppStyle.h"

#include <QApplication>
#include <QIcon>
#include <QFontMetrics>
#include <QDockWidget>
#include <QStyleFactory>
#include <QStyleOption>
#include <QStyleOptionDockWidget>

AppStyle::AppStyle()
    : QProxyStyle(QStyleFactory::create("fusion"))
{}

int AppStyle::pixelMetric(PixelMetric which,
    const QStyleOption *option, const QWidget *widget) const
{
    switch (which) {
    case PM_DockWidgetTitleMargin:
        return 6;
    default:
        return QProxyStyle::pixelMetric(which, option, widget);
    }
}

//static const QIcon appIcon            = QIcon(":/images/Corteva_64x.png");
//static const QIcon syncIcon           = QIcon(":/images/SyncArrow_32x.png");
//static const QIcon floatIcon          = QIcon(":/images/Restore_NoHalo_24x.png");
//static const QIcon closeIcon          = QIcon(":/images/Close_NoHalo_24x.png");
//static const QIcon functionIcon       = QIcon(":/images/Effects_32x.png"));
//static const QIcon sortAscendingIcon  = QIcon(":/images/SortAscending_16x.png");
//static const QIcon sortDescendingIcon = QIcon(":/images/SortDescending_16x.png");
//static const QIcon clearFilterIcon    = QIcon(":/images/DeleteFilter_16x.png");
//static const QIcon editFilterIcon     = QIcon(":/images/EditFilter_16x.png");
//static const QIcon fluxProfileIcon    = QIcon(":/images/KagiChart_32x.png");
//static const QIcon addIcon            = QIcon(":/images/Add_grey_16x.png");
//static const QIcon removeIcon         = QIcon(":/images/Remove_grey_16x.png");
//static const QIcon clearIcon          = QIcon(":/images/CleanData_24x.png");
//static const QIcon errorsIcon         = QIcon(":/images/StatusAnnotations_Critical_24xLG_color.png");
//static const QIcon warningsIcon       = QIcon(":/images/StatusAnnotations_Warning_24xLG_color.png");
//static const QIcon messagesIcon       = QIcon(":/images/StatusAnnotations_Information_24xLG_color.png");
//static const QIcon filterIcon         = QIcon(":/images/FilterDropdown_24x.png");
//static const QIcon debugIcon          = QIcon(":/images/StatusAnnotations_Information_16xLG.png");
//static const QIcon infoIcon           = QIcon(":/images/StatusAnnotations_Information_16xLG_color.png");
//static const QIcon warningIcon        = QIcon(":/images/StatusAnnotations_Warning_16xLG_color.png");
//static const QIcon errorIcon          = QIcon(":/images/StatusAnnotations_Invalid_16xLG_color.png");
//static const QIcon fatalIcon          = QIcon(":/images/StatusAnnotations_Critical_16xLG_color.png");
//static const QIcon editReceptorsIcon  = QIcon(":/images/EditReceptors_32x.png");
//static const QIcon newIcon            = QIcon(":/images/NewFile_40x.png");
//static const QIcon openIcon           = QIcon(":/images/OpenFolder_40x.png");
//static const QIcon saveIcon           = QIcon(":/images/Save_40x.png");
//static const QIcon saveAsIcon         = QIcon(":/images/SaveAs_32x.png");
//static const QIcon validateIcon       = QIcon(":/images/Checkmark_40x.png");
//static const QIcon runIcon            = QIcon(":/images/Run_40x.png");
//static const QIcon analyzeIcon        = QIcon(":/images/Measure_40x.png");
//static const QIcon helpIcon           = QIcon(":/images/MSHelpTableOfContent_32x.png");
//static const QIcon cloneIcon          = QIcon(":/images/CopyItem_16x.png");
//static const QIcon renameIcon         = QIcon(":/images/Rename_16x.png");
//static const QIcon addGroupIcon       = QIcon(":/images/AddBuildQueue_16x.png");
//static const QIcon importIcon         = QIcon(":/images/ImportFile_16x.png");
//static const QIcon receptorsIcon      = QIcon(":/images/EditReceptors_16x.png");
//static const QIcon tableIcon          = QIcon(":/images/Table_16x.png");
//static const QIcon textFileIcon       = QIcon(":/images/TextFile_16x.png");
//static const QIcon exportFileIcon     = QIcon(":/images/ExportFile_16x.png");
//static const QIcon sourceGroupIcon    = QIcon(":/images/BuildQueue_32x.png");
//static const QIcon settingsIcon       = QIcon(":/images/Settings_32x.png");
//static const QIcon runWindowIcon      = QIcon(":/images/ApplicationRunning_32x.png");
//static const QIcon areaSourceIcon     = QIcon(":/images/Rectangle_16x.png");
//static const QIcon circSourceIcon     = QIcon(":/images/Circle_16x.png");
//static const QIcon polySourceIcon     = QIcon(":/images/Polygon_16x.png");
//static const QIcon importIcon         = QIcon(":/images/Import_grey_16x.png");
//static const QIcon editIcon           = QIcon(":/images/Edit_grey_16x.png");
//static const QIcon colorIcon          = QIcon(":/images/ColorPalette_16x.png");
//static const QIcon resampleIcon       = QIcon(":/images/Refresh_16x.png");


QIcon AppStyle::standardIcon(QStyle::StandardPixmap icon,
    const QStyleOption *option, const QWidget *widget) const
{
    // Set custom float and close button icons.
    static const QIcon floatIcon = QIcon(":/images/Restore_NoHalo_24x.png");
    static const QIcon closeIcon = QIcon(":/images/Close_NoHalo_24x.png");

    switch (icon) {
    case QStyle::SP_TitleBarNormalButton:
        return floatIcon;
    case QStyle::SP_TitleBarCloseButton:
        return closeIcon;
    //case QStyle::SP_DialogCloseButton: // Tab Close Button
    //    return closeIcon;
    default:
        return QProxyStyle::standardIcon(icon, option, widget);
    }
}

void AppStyle::drawControl(QStyle::ControlElement element,
    const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionDockWidget *dwOption;
    const QDockWidget *dw;

    if (element == CE_DockWidgetTitle &&
       (dwOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) &&
       (dw = qobject_cast<const QDockWidget *>(widget)) &&
       !dw->titleBarWidget())
    {
        // Calculate the title bar height.
        int iconSize = dw->style()->pixelMetric(PM_SmallIconSize, nullptr, dw);
        int buttonMargin = dw->style()->pixelMetric(PM_DockWidgetTitleBarButtonMargin, nullptr, dw);
        int buttonHeight = iconSize + 2+buttonMargin;
        QFontMetrics titleFontMetrics = dw->fontMetrics();
        int titleMargin = dw->style()->pixelMetric(PM_DockWidgetTitleMargin, nullptr, dw);
        int titleHeight = qMax(buttonHeight + 2, titleFontMetrics.height() + 2*titleMargin);

        // Draw a background rectangle for the title bar.
        QColor bgColor = dwOption->palette.window().color().darker(104);
        QRect bgRect = dwOption->rect;
        bgRect.setHeight(titleHeight);
        painter->fillRect(bgRect, bgColor);

        // Add some left padding to the title text.
        QStyleOptionDockWidget customOption = *dwOption;
        customOption.rect.setLeft(5);

        QProxyStyle::drawControl(element, &customOption, painter, widget);
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void AppStyle::drawPrimitive(QStyle::PrimitiveElement element,
    const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int AppStyle::styleHint(StyleHint hint, const QStyleOption *option,
    const QWidget *widget, QStyleHintReturn *returnData) const
{
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QRect AppStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *option,
    QStyle::SubControl sc, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(cc, option, sc, widget);
}
