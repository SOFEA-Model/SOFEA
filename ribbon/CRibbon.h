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

#include <QLoggingCategory>
#include <QPalette>
#include <QString>
#include <QWidget>

#include <map>

#include <qt_windows.h>

#include <UIRibbon.h>

// Undocumented Property Keys
DEFINE_UIPROPERTYKEY(UI_PKEY_ApplicationButtonColor, VT_UI4, 2003);
DEFINE_UIPROPERTYKEY(UI_PKEY_DarkModeRibbon, VT_BOOL, 2004);

Q_DECLARE_LOGGING_CATEGORY(lcCRibbon)

enum class CRibbonProperty : unsigned long
{
    None                               = 0,
    Enabled                            = 1,
    LabelDescription                   = 2,
    Keytip                             = 3,
    Label                              = 4,
    TooltipDescription                 = 5,
    TooltipTitle                       = 6,
    LargeImage                         = 7,
    LargeHighContrastImage             = 8,
    SmallImage                         = 9,
    SmallHighContrastImage             = 10,
    CommandId                          = 100,
    ItemsSource                        = 101,
    Categories                         = 102,
    CategoryId                         = 103,
    SelectedItem                       = 104,
    CommandType                        = 105,
    ItemImage                          = 106,
    BooleanValue                       = 200,
    DecimalValue                       = 201,
    StringValue                        = 202,
    MaxValue                           = 203,
    MinValue                           = 204,
    Increment                          = 205,
    DecimalPlaces                      = 206,
    FormatString                       = 207,
    RepresentativeString               = 208,
    FontProperties                     = 300,
    FontProperties_Family              = 301,
    FontProperties_Size                = 302,
    FontProperties_Bold                = 303,
    FontProperties_Italic              = 304,
    FontProperties_Underline           = 305,
    FontProperties_Strikethrough       = 306,
    FontProperties_VerticalPositioning = 307,
    FontProperties_ForegroundColor     = 308,
    FontProperties_BackgroundColor     = 309,
    FontProperties_ForegroundColorType = 310,
    FontProperties_BackgroundColorType = 311,
    FontProperties_ChangedProperties   = 312,
    FontProperties_DeltaSize           = 313,
    RecentItems                        = 350,
    Pinned                             = 351,
    Color                              = 400,
    ColorType                          = 401,
    ColorMode                          = 402,
    ThemeColorsCategoryLabel           = 403,
    StandardColorsCategoryLabel        = 404,
    RecentColorsCategoryLabel          = 405,
    AutomaticColorLabel                = 406,
    NoColorLabel                       = 407,
    MoreColorsLabel                    = 408,
    ThemeColors                        = 409,
    StandardColors                     = 410,
    ThemeColorsTooltips                = 411,
    StandardColorsTooltips             = 412,
    Viewable                           = 1000,
    Minimized                          = 1001,
    QuickAccessToolbarDock             = 1002,
    ContextAvailable                   = 1100,
    GlobalBackgroundColor              = 2000,
    GlobalHighlightColor               = 2001,
    GlobalTextColor                    = 2002
};

struct CRibbonNotify
{
    // hdr.hwndFrom: top-level ribbon host window
    // hdr.idFrom: commandId (UINT32)
    // hdr.code: verb (UI_EXECUTIONVERB)
    NMHDR hdr;
    CRibbonProperty prop;
};

class CRibbon : public IUICommandHandler, public IUIApplication
{
public:
    explicit CRibbon(QWidget *frame);
    ~CRibbon() = default;
    CRibbon(const CRibbon&) = delete;
    CRibbon& operator=(const CRibbon&) = delete;

    HWND WindowHandle() const;
    int RibbonHeight() const;

    static UI_HSBCOLOR ColorToHSB(const QColor& color);
    static QColor HSBToColor(UI_HSBCOLOR hsb);
    bool SetHSBColor(QPalette::ColorRole role, UI_HSBCOLOR hsb);
    bool SetAccentColor(const QColor& color);
    QPalette Palette() const;
    bool SetPalette(const QPalette& Palette);

    bool CommandEnabled(UINT32 commandId) const;
    void SetCommandEnabled(UINT32 commandId, bool enabled);
    bool CommandChecked(UINT32 commandId) const;
    void SetCommandChecked(UINT32 commandId, bool checked);
    UI_CONTEXTAVAILABILITY TabGroupContextAvailability(UINT32 commandId);
    void SetTabGroupContextAvailability(UINT32 commandId, UI_CONTEXTAVAILABILITY state);
    UI_SWATCHCOLORTYPE DDCPColorType(UINT32 commandId) const;
    void SetDDCPColorType(UINT32 commandId, UI_SWATCHCOLORTYPE colorType);
    QColor DDCPColor(UINT32 commandId) const;
    void SetDDCPColor(UINT32 commandId, const QColor& color);

    // IUIFramework
    bool SetModes(UINT modes);
    bool FlushPendingInvalidations();
    bool GetUICommandProperty(UINT32 commandId, const PROPERTYKEY& key, PROPVARIANT *value) const;
    bool SetUICommandProperty(UINT32 commandId, const PROPERTYKEY& key, const PROPVARIANT& value);
    bool InvalidateUICommand(UINT32 commandId, UI_INVALIDATIONS flags, const PROPERTYKEY *key);

private:
    // IUnknown
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;
    HRESULT __stdcall QueryInterface(REFIID iid, void **ppv) override;

    // IUIApplication
    HRESULT __stdcall OnCreateUICommand(UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler **ppCommandHandler) override;
    HRESULT __stdcall OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler *pCommandHandler) override;
    HRESULT __stdcall OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32 reasonCode) override;

    // IUICommandHandler
    HRESULT __stdcall UpdateProperty(UINT32 commandId, const PROPERTYKEY& key, const PROPVARIANT *currentValue, PROPVARIANT *newValue) override;
    HRESULT __stdcall Execute(UINT32 commandId, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *value, IUISimplePropertySet *executionProperties) override;

    void UpdateRibbonHeight();
    void EraseRibbonBackground();
    void UpdateFrameGeometry();

    static LRESULT __stdcall WndProc(HWND hWnd, UINT32 message, WPARAM wParam, LPARAM lParam);

    IUIFramework *pFramework_ = nullptr;
    LONG cRef_ = 0;
    QWidget *frame_ = nullptr;
    HWND hWnd_ = nullptr;
    HWND hWndFrame_ = nullptr;
    int ribbonHeight_ = 0;
    int pixelRatio_ = 1;
};
