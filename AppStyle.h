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

#pragma once

#include <QIcon>
#include <QPalette>
#include <QStyle>
#include <QProxyStyle>

QT_BEGIN_NAMESPACE
class QStyleOption;
class QStyleOptionComplex;
class QWidget;
class QPainter;
QT_END_NAMESPACE

class AppStyle : public QProxyStyle
{
    Q_OBJECT

public:
    AppStyle();

    int pixelMetric(PixelMetric which, const QStyleOption *option,
        const QWidget *widget) const override;
    QIcon standardIcon(QStyle::StandardPixmap sp, const QStyleOption *option = nullptr,
        const QWidget *widget = nullptr) const override;
    QPalette standardPalette() const override;
    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option,
        QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option,
        QPainter *painter, const QWidget *widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
        const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;

    enum CustomPixmap {
        CP_NewFile = QStyle::SP_CustomBase + 1,
        CP_NewFolder,
        CP_OpenFile,
        CP_OpenFolder,
        CP_Save,
        CP_SaveAs,
        CP_Checkmark,
        CP_Run,
        CP_RunSettings,
        CP_Measure,
        CP_StatusHelp,
        CP_StatusOK,
        CP_StatusInfoTip,
        CP_StatusRequired,
        CP_StatusDebug,
        CP_StatusInformation,
        CP_StatusAlert,
        CP_StatusWarning,
        CP_StatusInvalid,
        CP_StatusCritical,
        CP_HeaderSortAscending,
        CP_HeaderSortDescending,
        CP_HeaderEditFilter,
        CP_HeaderDeleteFilter,
        CP_CalculateButton,
        CP_FilterDropdownButton,
        CP_LineEditFunctionIcon,
        CP_SyncArrow,
        CP_WindRose,
        CP_ActionAdd,
        CP_ActionRemove,
        CP_ActionEdit,
        CP_ActionClone,
        CP_ActionRename,
        CP_ActionCleanData,
        CP_ActionImport,
        CP_ActionImportFile,
        CP_ActionExport,
        CP_ActionExportData,
        CP_ActionExportFile,
        CP_ActionExportToExcel,
        CP_ActionGenerateFile,
        CP_ActionGenerateTable,
        CP_ActionTable,
        CP_ActionTableFunction,
        CP_ActionTextFile,
        CP_ActionShowDataPreview,
        CP_ActionRefresh,
        CP_ActionSettings,
        CP_ActionEditorZone,
        CP_ActionAddArea,
        CP_ActionAddCircular,
        CP_ActionAddPolygon,
        CP_ActionAddBuildQueue,
        CP_ActionStepChart,
        CP_ActionColorPalette
    };

    Q_ENUM(CustomPixmap)
};
