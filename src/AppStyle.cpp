// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "AppStyle.h"

#include <QApplication>
#include <QColor>
#include <QDockWidget>
#include <QFontMetrics>
#include <QGroupBox>
#include <QPainter>
#include <QRect>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QToolBox>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

AppStyle::AppStyle()
    : QProxyStyle(QStyleFactory::create("fusion"))
{}

int AppStyle::pixelMetric(PixelMetric which,
    const QStyleOption *option, const QWidget *widget) const
{
    switch (which) {
    case PM_DockWidgetTitleMargin:
        return 6;
    case PM_ToolBarIconSize:
        return QProxyStyle::pixelMetric(PM_SmallIconSize, option, widget);
    default:
        return QProxyStyle::pixelMetric(which, option, widget);
    }
}

inline QIcon smallIconResource(const QString& baseName, const QString& suffix = "")
{
    QIcon icon;
    icon.addFile(QString(":/res/%1_16x%2.png").arg(baseName).arg(suffix), QSize(16, 16));
    icon.addFile(QString(":/res/%1_20x%2.png").arg(baseName).arg(suffix), QSize(20, 20));
    icon.addFile(QString(":/res/%1_24x%2.png").arg(baseName).arg(suffix), QSize(24, 24));
    icon.addFile(QString(":/res/%1_32x%2.png").arg(baseName).arg(suffix), QSize(32, 32));
    return icon;
}

inline QIcon largeIconResource(const QString& baseName, const QString& suffix = "")
{
    QIcon icon;
    icon.addFile(QString(":/res/%1_16x%2.png").arg(baseName).arg(suffix), QSize(16, 16));
    icon.addFile(QString(":/res/%1_20x%2.png").arg(baseName).arg(suffix), QSize(20, 20));
    icon.addFile(QString(":/res/%1_24x%2.png").arg(baseName).arg(suffix), QSize(24, 24));
    icon.addFile(QString(":/res/%1_32x%2.png").arg(baseName).arg(suffix), QSize(32, 32));
    icon.addFile(QString(":/res/%1_36x%2.png").arg(baseName).arg(suffix), QSize(36, 36));
    icon.addFile(QString(":/res/%1_40x%2.png").arg(baseName).arg(suffix), QSize(40, 40));
    icon.addFile(QString(":/res/%1_48x%2.png").arg(baseName).arg(suffix), QSize(48, 48));
    icon.addFile(QString(":/res/%1_64x%2.png").arg(baseName).arg(suffix), QSize(64, 64));
    return icon;
}

QIcon AppStyle::standardIcon(QStyle::StandardPixmap sp,
    const QStyleOption *option, const QWidget *widget) const
{   
    // QDockWidget Icons
    static const QIcon floatIcon             = smallIconResource("Undock_NoHalo");
    static const QIcon closeIcon             = smallIconResource("Close_NoHalo");
    // Toolbar Icons
    static const QIcon newFileIcon           = largeIconResource("NewFile");
    static const QIcon newFolderIcon         = largeIconResource("NewSolutionFolder");
    static const QIcon openFileIcon          = largeIconResource("OpenFile");
    static const QIcon openFolderIcon        = largeIconResource("OpenFolder");
    static const QIcon saveIcon              = largeIconResource("Save");
    static const QIcon saveAsIcon            = largeIconResource("SaveAs");
    static const QIcon checkmarkIcon         = largeIconResource("Checkmark");
    static const QIcon runIcon               = largeIconResource("Run");
    static const QIcon runSettingsIcon       = largeIconResource("RunTestDialog");
    static const QIcon measureIcon           = largeIconResource("Measure");
    // Status Icons
    static const QIcon statusHelpIcon        = largeIconResource("StatusHelp");
    static const QIcon statusOkIcon          = smallIconResource("StatusOK");
    static const QIcon statusInfoTipIcon     = smallIconResource("StatusInfoTip");
    static const QIcon statusRequiredIcon    = smallIconResource("StatusRequired");
    static const QIcon statusInformationIcon = smallIconResource("StatusInformation");
    static const QIcon statusAlertIcon       = smallIconResource("StatusAlert");
    static const QIcon statusWarningIcon     = smallIconResource("StatusWarning");
    static const QIcon statusInvalidIcon     = smallIconResource("StatusInvalid");
    static const QIcon statusCriticalIcon    = smallIconResource("StatusCriticalError");
    // Header View Icons
    static const QIcon sortAscendingIcon     = smallIconResource("SortAscending");
    static const QIcon sortDescendingIcon    = smallIconResource("SortDescending");
    static const QIcon editFilterIcon        = smallIconResource("EditFilter");
    static const QIcon deleteFilterIcon      = smallIconResource("DeleteFilter");
    static const QIcon checkboxCheckAllIcon  = smallIconResource("CheckboxCheckAll");
    static const QIcon checkboxClearAllIcon  = smallIconResource("CheckboxClearAll");
    // Button Icons
    static const QIcon calculateIcon         = smallIconResource("Calculator");
    static const QIcon filterDropdownIcon    = smallIconResource("FilterDropdown");
    static const QIcon functionIcon          = smallIconResource("Effects");
    static const QIcon syncArrowIcon         = smallIconResource("SyncArrow");
    static const QIcon windRoseIcon          = smallIconResource("WindRose");
    // Action Icons
    static const QIcon addIcon               = smallIconResource("Add", "MD");
    static const QIcon removeIcon            = smallIconResource("Remove", "MD");
    static const QIcon editIcon              = smallIconResource("Edit");
    static const QIcon cloneIcon             = smallIconResource("CopyItem");
    static const QIcon renameIcon            = smallIconResource("Rename");
    static const QIcon cleanDataIcon         = smallIconResource("CleanData");
    static const QIcon importIcon            = smallIconResource("Import");
    static const QIcon importFileIcon        = smallIconResource("ImportFile");
    static const QIcon exportIcon            = smallIconResource("Export");
    static const QIcon exportDataIcon        = smallIconResource("ExportData");
    static const QIcon exportFileIcon        = smallIconResource("ExportFile");
    static const QIcon exportToExcelIcon     = smallIconResource("ExportToExcel");
    static const QIcon generateFileIcon      = smallIconResource("GenerateFile");
    static const QIcon generateTableIcon     = smallIconResource("GenerateTable");
    static const QIcon tableIcon             = smallIconResource("Table");
    static const QIcon tableFunctionIcon     = smallIconResource("TableFunction");
    static const QIcon textFileIcon          = smallIconResource("TextFile");
    static const QIcon showDataPreviewIcon   = smallIconResource("ShowDataPreview");
    static const QIcon refreshIcon           = smallIconResource("Refresh");
    static const QIcon settingsIcon          = smallIconResource("Settings");
    static const QIcon editorZoneIcon        = smallIconResource("EditorZone");
    static const QIcon addAreaIcon           = smallIconResource("AddArea");
    static const QIcon addCircularIcon       = smallIconResource("AddCircular");
    static const QIcon addPolygonIcon        = smallIconResource("AddPolygon");
    static const QIcon addBuildQueueIcon     = smallIconResource("AddBuildQueue");
    static const QIcon stepChartIcon         = smallIconResource("KagiChart");
    static const QIcon colorPaletteIcon      = smallIconResource("ColorPalette");

    int index = static_cast<int>(sp);
    switch (index) {
    case QStyle::SP_TitleBarNormalButton: // DockWidget Float Button
        return floatIcon;
    case QStyle::SP_TitleBarCloseButton: // DockWidget Close Button
        return closeIcon;
    case QStyle::SP_DialogCloseButton: // Tab Close Button
        return closeIcon;
    case AppStyle::CP_NewFile:
        return newFileIcon;
    case AppStyle::CP_NewFolder:
        return newFolderIcon;
    case AppStyle::CP_OpenFile:
        return openFileIcon;
    case AppStyle::CP_OpenFolder:
        return openFolderIcon;
    case AppStyle::CP_Save:
        return saveIcon;
    case AppStyle::CP_SaveAs:
        return saveAsIcon;
    case AppStyle::CP_Checkmark:
        return checkmarkIcon;
    case AppStyle::CP_Run:
        return runIcon;
    case AppStyle::CP_RunSettings:
        return runSettingsIcon;
    case AppStyle::CP_Measure:
        return measureIcon;
    case AppStyle::CP_StatusHelp:
        return statusHelpIcon;
    case AppStyle::CP_StatusOK:
        return statusOkIcon;
    case AppStyle::CP_StatusInfoTip:
        return statusInfoTipIcon;
    case AppStyle::CP_StatusRequired:
        return statusRequiredIcon;
    case AppStyle::CP_StatusDebug:
        return statusInformationIcon;
    case AppStyle::CP_StatusInformation:
        return statusInformationIcon;
    case AppStyle::CP_StatusAlert:
        return statusAlertIcon;
    case AppStyle::CP_StatusWarning:
        return statusWarningIcon;
    case AppStyle::CP_StatusInvalid:
        return statusInvalidIcon;
    case AppStyle::CP_StatusCritical:
        return statusCriticalIcon;
    case AppStyle::CP_HeaderSortAscending:
        return sortAscendingIcon;
    case AppStyle::CP_HeaderSortDescending:
        return sortDescendingIcon;
    case AppStyle::CP_HeaderEditFilter:
        return editFilterIcon;
    case AppStyle::CP_HeaderDeleteFilter:
        return deleteFilterIcon;
    case AppStyle::CP_CheckboxCheckAll:
        return checkboxCheckAllIcon;
    case AppStyle::CP_CheckboxClearAll:
        return checkboxClearAllIcon;
    case AppStyle::CP_CalculateButton:
        return calculateIcon;
    case AppStyle::CP_FilterDropdownButton:
        return filterDropdownIcon;
    case AppStyle::CP_LineEditFunctionIcon:
        return functionIcon;
    case AppStyle::CP_SyncArrow:
        return syncArrowIcon;
    case AppStyle::CP_WindRose:
        return windRoseIcon;
    case AppStyle::CP_ActionAdd:
        return addIcon;
    case AppStyle::CP_ActionRemove:
        return removeIcon;
    case AppStyle::CP_ActionEdit:
        return editIcon;
    case AppStyle::CP_ActionClone:
        return cloneIcon;
    case AppStyle::CP_ActionRename:
        return renameIcon;
    case AppStyle::CP_ActionCleanData:
        return cleanDataIcon;
    case AppStyle::CP_ActionImport:
        return importIcon;
    case AppStyle::CP_ActionImportFile:
        return importFileIcon;
    case AppStyle::CP_ActionExport:
        return exportIcon;
    case AppStyle::CP_ActionExportData:
        return exportDataIcon;
    case AppStyle::CP_ActionExportFile:
        return exportFileIcon;
    case AppStyle::CP_ActionExportToExcel:
        return exportToExcelIcon;
    case AppStyle::CP_ActionGenerateFile:
        return generateFileIcon;
    case AppStyle::CP_ActionGenerateTable:
        return generateTableIcon;
    case AppStyle::CP_ActionTable:
        return tableIcon;
    case AppStyle::CP_ActionTableFunction:
        return tableFunctionIcon;
    case AppStyle::CP_ActionTextFile:
        return textFileIcon;
    case AppStyle::CP_ActionShowDataPreview:
        return showDataPreviewIcon;
    case AppStyle::CP_ActionRefresh:
        return refreshIcon;
    case AppStyle::CP_ActionSettings:
        return settingsIcon;
    case AppStyle::CP_ActionEditorZone:
        return editorZoneIcon;
    case AppStyle::CP_ActionAddArea:
        return addAreaIcon;
    case AppStyle::CP_ActionAddCircular:
        return addCircularIcon;
    case AppStyle::CP_ActionAddPolygon:
        return addPolygonIcon;
    case AppStyle::CP_ActionAddBuildQueue:
        return addBuildQueueIcon;
    case AppStyle::CP_ActionStepChart:
        return stepChartIcon;
    case AppStyle::CP_ActionColorPalette:
        return colorPaletteIcon;
    default:
        return QProxyStyle::standardIcon(sp, option, widget);
    }
}

QPalette AppStyle::standardPalette() const
{
    // Dark Palette
    //QPalette p = QProxyStyle::standardPalette();
    //p.setColor(QPalette::Window, QColor(53,53,53));
    //p.setColor(QPalette::WindowText, Qt::white);
    //p.setColor(QPalette::Base, QColor(42,42,42));
    //p.setColor(QPalette::AlternateBase, QColor(66,66,66));
    //p.setColor(QPalette::ToolTipBase, Qt::white);
    //p.setColor(QPalette::ToolTipText, QColor(53,53,53));
    //p.setColor(QPalette::Text, Qt::white);
    //p.setColor(QPalette::Light, QColor(71,71,71));
    //p.setColor(QPalette::Midlight, QColor(62,62,62));
    //p.setColor(QPalette::Dark, QColor(35,35,35));
    //p.setColor(QPalette::Mid, QColor(44,44,44));
    //p.setColor(QPalette::Shadow, QColor(20,20,20));
    //p.setColor(QPalette::Button, QColor(53,53,53));
    //p.setColor(QPalette::ButtonText, Qt::white);
    //p.setColor(QPalette::BrightText, Qt::red);
    //p.setColor(QPalette::Link, QColor(42,130,218));
    //p.setColor(QPalette::Highlight, QColor(42,130,218));
    //p.setColor(QPalette::HighlightedText, Qt::white);
    //p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127,127,127));
    //p.setColor(QPalette::Disabled, QPalette::Text, QColor(127,127,127));
    //p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127,127,127));
    //p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80,80,80));
    //p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127,127,127));
    //return p;

    //QPalette p = QProxyStyle::standardPalette();
    //p.setColor(QPalette::Window, QColor(245, 246, 247));
    //p.setColor(QPalette::Button, QColor(245, 246, 247));
    //return p;

    return QProxyStyle::standardPalette();
}

void AppStyle::drawComplexControl(QStyle::ComplexControl control,
    const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control)
    {
    case CC_ToolButton:
        // Workaround for QTBUG-2036: https://bugreports.qt.io/browse/QTBUG-2036
        const QStyleOptionToolButton *tbOption;
        const QToolButton *tb;
        if ((tbOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) &&
           (tb = qobject_cast<const QToolButton *>(widget)) &&
           tb->arrowType() == Qt::NoArrow)
        {
            QStyleOptionToolButton customOption = *tbOption;
            customOption.features &= (~QStyleOptionToolButton::HasMenu);
            QProxyStyle::drawComplexControl(control, &customOption, painter, widget);
            return;
        }
        break;
    case CC_GroupBox:
        // Draw a custom header for QGroupBox.
        /*
        const QStyleOptionGroupBox *gbopt;
        const QGroupBox *gb;
        if ((gbopt = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) &&
           (gb = qobject_cast<const QGroupBox *>(widget)))
        {
            painter->save();
            QRect textRect = proxy()->subControlRect(CC_GroupBox, gbopt, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, gbopt, SC_GroupBoxCheckBox, widget);

            // Draw a custom frame.
            const int x1 = gbopt->rect.left();
            const int y1 = gbopt->rect.top();
            const int x2 = gbopt->rect.right();
            const int y2 = textRect.bottom();
            QRect headerRect = QRect(QPoint{x1, y1}, QPoint{x2, y2});

            painter->fillRect(headerRect, gbopt->palette.button());

            QStyleOptionFrame fropt;
            fropt.QStyleOption::operator=(*gbopt);
            fropt.features = QStyleOptionFrame::Rounded;
            fropt.lineWidth = gbopt->lineWidth;
            fropt.midLineWidth = gbopt->midLineWidth;
            fropt.rect = headerRect;

            proxy()->drawPrimitive(PE_Frame, &fropt, painter, widget);

            // Adjust the rects for the frame.
            checkBoxRect.moveLeft(checkBoxRect.left() + 5);
            textRect.moveLeft(textRect.left() + 5);

            // Draw title.
            if ((gbopt->subControls & QStyle::SC_GroupBoxLabel) && !gbopt->text.isEmpty()) {
                 painter->setPen(QPen(gbopt->palette.windowText(), 1));
                 proxy()->drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignLeft,
                                       gbopt->palette, gbopt->state & State_Enabled, gbopt->text, QPalette::NoRole);
            }

            // Draw indicator.
            if (gbopt->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*gbopt);
                box.rect = checkBoxRect;
                proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }

            painter->restore();
            return;
        }
        */
        break;
    default:
        break;
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void AppStyle::drawControl(QStyle::ControlElement element,
    const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case CE_DockWidgetTitle:
        // Customize the QDockWidget title bar.
        const QStyleOptionDockWidget *dwopt;
        const QDockWidget *dw;
        if ((dwopt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) &&
           (dw = qobject_cast<const QDockWidget *>(widget)) &&
           !dw->titleBarWidget())
        {
            // Calculate the title bar height.
            int iconSize = dw->style()->pixelMetric(PM_SmallIconSize, nullptr, dw);
            int buttonMargin = dw->style()->pixelMetric(PM_DockWidgetTitleBarButtonMargin, nullptr, dw);
            int titleMargin = dw->style()->pixelMetric(PM_DockWidgetTitleMargin, nullptr, dw);
            QFontMetrics titleFontMetrics = dw->fontMetrics();
            int buttonHeight = iconSize + 2 + buttonMargin;
            int titleHeight = qMax(buttonHeight + 2, titleFontMetrics.height() + 2 * titleMargin);

            // Draw a background rectangle for the title bar.
            QColor fillColor = dwopt->palette.window().color().darker(106);
            QRect titleRect = dwopt->rect;
            titleRect.setHeight(titleHeight);
            painter->fillRect(titleRect, fillColor);

            // Add some left padding to the title text.
            QStyleOptionDockWidget customOption = *dwopt;
            customOption.rect.setLeft(titleMargin);

            QProxyStyle::drawControl(element, &customOption, painter, widget);
            return;
        }
        break;
    case CE_ToolBoxTab:
        // Draw the QToolBox title bar using a QTabBar frame.
        const QStyleOptionToolBox *tbxopt;
        if ((tbxopt = qstyleoption_cast<const QStyleOptionToolBox *>(option)))
        {
            QStyleOptionTab tabopt;
            tabopt.rect = tbxopt->rect;
            tabopt.state = tbxopt->state;
            tabopt.palette = tbxopt->palette;
            tabopt.fontMetrics = tbxopt->fontMetrics;
            tabopt.text = tbxopt->text;
            tabopt.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tabopt.position = QStyleOptionTab::OnlyOneTab;
            tabopt.selectedPosition = QStyleOptionTab::NotAdjacent;
            tabopt.shape = QTabBar::RoundedNorth;
            tabopt.features = QStyleOptionTab::None;

            QProxyStyle::drawControl(CE_TabBarTabShape, &tabopt, painter, widget);
            QProxyStyle::drawControl(CE_TabBarTabLabel, &tabopt, painter, widget);
            return;
        }
        break;
    case CE_ItemViewItem:
        // Check whether to draw selections using active state.
        if (this->styleHint(SH_ItemView_ChangeHighlightOnFocus, option, widget) == 0) {
            const QStyleOptionViewItem *itemopt;
            if ((itemopt = qstyleoption_cast<const QStyleOptionViewItem *>(option)) &&
                (itemopt->state & QStyle::State_Selected))
            {
                QStyleOptionViewItem customOption = *itemopt;
                customOption.state |= QStyle::State_Active | QStyle::State_HasFocus;
                QProxyStyle::drawControl(element, &customOption, painter, widget);
                return;
            }
        }
        break;
    default:
        break;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}


void AppStyle::drawPrimitive(QStyle::PrimitiveElement element,
    const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case PE_PanelItemViewRow:
        // Check whether to draw selections using active state.
        if (this->styleHint(SH_ItemView_ChangeHighlightOnFocus, option, widget) == 0) {
            const QStyleOptionViewItem *itemopt;
            if ((itemopt = qstyleoption_cast<const QStyleOptionViewItem *>(option)) &&
                (itemopt->state & QStyle::State_Selected))
            {
                QStyleOptionViewItem customOption = *itemopt;
                customOption.state |= QStyle::State_Active | QStyle::State_HasFocus;
                QProxyStyle::drawPrimitive(element, &customOption, painter, widget);
                return;
            }
        }
        break;
    default:
        break;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

int AppStyle::styleHint(StyleHint hint, const QStyleOption *option,
    const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint)
    {
    case SH_Widget_Animation_Duration:
        // Disable animations.
        return 0;
    case SH_ItemView_ChangeHighlightOnFocus:
        // Keep QTreeView items highlighted when losing focus.
        if (const QTreeView *treeview = qobject_cast<const QTreeView *>(widget))
            return 0;
        else
            return 1;
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

QRect AppStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *option,
    QStyle::SubControl sc, const QWidget *widget) const
{
    return QProxyStyle::subControlRect(cc, option, sc, widget);
}
