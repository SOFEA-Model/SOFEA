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

#include "CRibbon.h"

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QWindow>
#include <iostream>
#include <cmath>

#include <UIRibbonPropertyHelpers.h>

Q_LOGGING_CATEGORY(lcCRibbon, "CRibbon", QtWarningMsg)

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

inline long long QueryMilliseconds()
{
    static LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);
    return (1000LL * counter.QuadPart) / frequency.QuadPart;
}

inline unsigned long UIPropertyKeyIndex(const PROPERTYKEY *pKey)
{
    if (!pKey)
        return 0;

    // Make sure this is a TUIPROPERTYKEY. See DEFINE_UIPROPERTYKEY macro in UIRibbonKeydef.h
    const GUID uiPropertyKeyGUID = { pKey->fmtid.Data1, 0x7363, 0x696e, { 0x84, 0x41, 0x79, 0x8a, 0xcf, 0x5a, 0xeb, 0xb7 } };
    if (!InlineIsEqualGUID(pKey->fmtid, uiPropertyKeyGUID))
        return 0;

    return pKey->fmtid.Data1;
}

inline const char* UIPropertyKeyStr(const PROPERTYKEY& key)
{
    const unsigned long index = UIPropertyKeyIndex(&key);

    switch (index) {
    case 0:    return "NULL";
    case 1:    return "UI_PKEY_Enabled";
    case 2:    return "UI_PKEY_LabelDescription";
    case 3:    return "UI_PKEY_Keytip";
    case 4:    return "UI_PKEY_Label";
    case 5:    return "UI_PKEY_TooltipDescription";
    case 6:    return "UI_PKEY_TooltipTitle";
    case 7:    return "UI_PKEY_LargeImage";
    case 8:    return "UI_PKEY_LargeHighContrastImage";
    case 9:    return "UI_PKEY_SmallImage";
    case 10:   return "UI_PKEY_SmallHighContrastImage";
    case 100:  return "UI_PKEY_CommandId";
    case 101:  return "UI_PKEY_ItemsSource";
    case 102:  return "UI_PKEY_Categories";
    case 103:  return "UI_PKEY_CategoryId";
    case 104:  return "UI_PKEY_SelectedItem";
    case 105:  return "UI_PKEY_CommandType";
    case 106:  return "UI_PKEY_ItemImage";
    case 200:  return "UI_PKEY_BooleanValue";
    case 201:  return "UI_PKEY_DecimalValue";
    case 202:  return "UI_PKEY_StringValue";
    case 203:  return "UI_PKEY_MaxValue";
    case 204:  return "UI_PKEY_MinValue";
    case 205:  return "UI_PKEY_Increment";
    case 206:  return "UI_PKEY_DecimalPlaces";
    case 207:  return "UI_PKEY_FormatString";
    case 208:  return "UI_PKEY_RepresentativeString";
    case 300:  return "UI_PKEY_FontProperties";
    case 301:  return "UI_PKEY_FontProperties_Family";
    case 302:  return "UI_PKEY_FontProperties_Size";
    case 303:  return "UI_PKEY_FontProperties_Bold";
    case 304:  return "UI_PKEY_FontProperties_Italic";
    case 305:  return "UI_PKEY_FontProperties_Underline";
    case 306:  return "UI_PKEY_FontProperties_Strikethrough";
    case 307:  return "UI_PKEY_FontProperties_VerticalPositioning";
    case 308:  return "UI_PKEY_FontProperties_ForegroundColor";
    case 309:  return "UI_PKEY_FontProperties_BackgroundColor";
    case 310:  return "UI_PKEY_FontProperties_ForegroundColorType";
    case 311:  return "UI_PKEY_FontProperties_BackgroundColorType";
    case 312:  return "UI_PKEY_FontProperties_ChangedProperties";
    case 313:  return "UI_PKEY_FontProperties_DeltaSize";
    case 350:  return "UI_PKEY_RecentItems";
    case 351:  return "UI_PKEY_Pinned";
    case 400:  return "UI_PKEY_Color";
    case 401:  return "UI_PKEY_ColorType";
    case 402:  return "UI_PKEY_ColorMode";
    case 403:  return "UI_PKEY_ThemeColorsCategoryLabel";
    case 404:  return "UI_PKEY_StandardColorsCategoryLabel";
    case 405:  return "UI_PKEY_RecentColorsCategoryLabel";
    case 406:  return "UI_PKEY_AutomaticColorLabel";
    case 407:  return "UI_PKEY_NoColorLabel";
    case 408:  return "UI_PKEY_MoreColorsLabel";
    case 409:  return "UI_PKEY_ThemeColors";
    case 410:  return "UI_PKEY_StandardColors";
    case 411:  return "UI_PKEY_ThemeColorsTooltips";
    case 412:  return "UI_PKEY_StandardColorsTooltips";
    case 1000: return "UI_PKEY_Viewable";
    case 1001: return "UI_PKEY_Minimized";
    case 1002: return "UI_PKEY_QuickAccessToolbarDock";
    case 1100: return "UI_PKEY_ContextAvailable";
    case 2000: return "UI_PKEY_GlobalBackgroundColor";
    case 2001: return "UI_PKEY_GlobalHighlightColor";
    case 2002: return "UI_PKEY_GlobalTextColor";
    default:   return "INVALID";
    }
}

inline const char* UICommandTypeStr(UI_COMMANDTYPE typeId)
{
    switch (typeId) {
    case UI_COMMANDTYPE_UNKNOWN:           return "UI_COMMANDTYPE_UNKNOWN";
    case UI_COMMANDTYPE_GROUP:             return "UI_COMMANDTYPE_GROUP";
    case UI_COMMANDTYPE_ACTION:            return "UI_COMMANDTYPE_ACTION";
    case UI_COMMANDTYPE_ANCHOR:            return "UI_COMMANDTYPE_ANCHOR";
    case UI_COMMANDTYPE_CONTEXT:           return "UI_COMMANDTYPE_CONTEXT";
    case UI_COMMANDTYPE_COLLECTION:        return "UI_COMMANDTYPE_COLLECTION";
    case UI_COMMANDTYPE_COMMANDCOLLECTION: return "UI_COMMANDTYPE_COMMANDCOLLECTION";
    case UI_COMMANDTYPE_DECIMAL:           return "UI_COMMANDTYPE_DECIMAL";
    case UI_COMMANDTYPE_BOOLEAN:           return "UI_COMMANDTYPE_BOOLEAN";
    case UI_COMMANDTYPE_FONT:              return "UI_COMMANDTYPE_FONT";
    case UI_COMMANDTYPE_RECENTITEMS:       return "UI_COMMANDTYPE_RECENTITEMS";
    case UI_COMMANDTYPE_COLORANCHOR:       return "UI_COMMANDTYPE_COLORANCHOR";
    case UI_COMMANDTYPE_COLORCOLLECTION:   return "UI_COMMANDTYPE_COLORCOLLECTION";
    default:                               return "INVALID";
    }
}

inline const char* UIViewVerbStr(UI_VIEWVERB verb)
{
    switch (verb) {
    case UI_VIEWVERB_CREATE:  return "UI_VIEWVERB_CREATE";
    case UI_VIEWVERB_SIZE:    return "UI_VIEWVERB_SIZE";
    case UI_VIEWVERB_DESTROY: return "UI_VIEWVERB_DESTROY";
    case UI_VIEWVERB_ERROR:   return "UI_VIEWVERB_ERROR";
    default:                  return "INVALID";
    }
}

inline const char* UIExecutionVerbStr(UI_EXECUTIONVERB verb)
{
    switch (verb) {
    case UI_EXECUTIONVERB_EXECUTE:       return "UI_EXECUTIONVERB_EXECUTE";
    case UI_EXECUTIONVERB_PREVIEW:       return "UI_EXECUTIONVERB_PREVIEW";
    case UI_EXECUTIONVERB_CANCELPREVIEW: return "UI_EXECUTIONVERB_CANCELPREVIEW";
    default:                             return "INVALID";
    }
}


inline const char* UIContextAvailabilityStr(UI_CONTEXTAVAILABILITY state)
{
    switch (state) {
    case UI_CONTEXTAVAILABILITY_NOTAVAILABLE: return "UI_CONTEXTAVAILABILITY_NOTAVAILABLE";
    case UI_CONTEXTAVAILABILITY_AVAILABLE:    return "UI_CONTEXTAVAILABILITY_AVAILABLE";
    case UI_CONTEXTAVAILABILITY_ACTIVE:       return "UI_CONTEXTAVAILABILITY_ACTIVE";
    default:                                  return "INVALID";
    }
}

inline bool UIPropertyValue(const TUIPROPERTYKEY<VT_BOOL>& key, const PROPVARIANT& pv)
{
    BOOL result;
    if (SUCCEEDED(UIPropertyToBoolean(key, pv, &result)))
        return (result == TRUE);
    return false;
}

inline unsigned int UIPropertyValue(const TUIPROPERTYKEY<VT_UI4>& key, const PROPVARIANT& pv)
{
    UINT result;
    if (SUCCEEDED(UIPropertyToUInt32(key, pv, &result)))
        return result;
    return 0;
}

inline QString UIPropertyValue(const TUIPROPERTYKEY<VT_LPWSTR>&, const PROPVARIANT& pv)
{
    if (pv.vt == VT_LPWSTR)
        return QString::fromUtf16(reinterpret_cast<const USHORT *>(pv.pwszVal));
    return QString();
}

inline double UIPropertyValue(const TUIPROPERTYKEY<VT_DECIMAL>&, const PROPVARIANT& pv)
{
    double result = 0;
    if (pv.vt == VT_DECIMAL)
        ::VarR8FromDec(&pv.decVal, &result);
    return result;
}

//------------------------------------------------------------------------------
// CRibbon
//------------------------------------------------------------------------------

CRibbon::CRibbon(QWidget *frame)
{
    // Set the QWidget to receive command messages and geometry updates.
    frame_ = frame;
    pixelRatio_ = frame_->devicePixelRatio();
    hWndFrame_ = reinterpret_cast<HWND>(frame_->winId());

    // Create the ribbon host window.
    HINSTANCE hInstance = ::GetModuleHandle(nullptr);
    WNDCLASSEX wcex = { 0 };

    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = 0; // Don't use CS_HREDRAW or CS_VREDRAW with a Ribbon
    wcex.lpfnWndProc   = this->WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = ::LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = nullptr;
    wcex.lpszClassName = L"CRibbon";
    wcex.hIconSm       = ::LoadIcon(nullptr, IDI_APPLICATION);

    ::RegisterClassEx(&wcex);

    hWnd_ = ::CreateWindowEx(0, L"CRibbon", L"", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd_) {
        qCCritical(lcCRibbon, "CreateWindowEx failed");
        return;
    }

    ::SetWindowLongPtr(hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Initialize the ribbon framework.
    if (FAILED(::CoCreateInstance(CLSID_UIRibbonFramework, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFramework_)))) {
        qCCritical(lcCRibbon, "CoCreateInstance(CLSID_UIRibbonFramework) failed");
        return;
    }

    if (FAILED(pFramework_->Initialize(hWnd_, this))) {
        qCCritical(lcCRibbon, "IUIFramework::Initialize failed");
        return;
    }

    if (FAILED(pFramework_->LoadUI(::GetModuleHandle(nullptr), L"APPLICATION_RIBBON"))) {
        qCCritical(lcCRibbon, "IUIFramework::LoadUI failed");
        return;
    }
}

HWND CRibbon::WindowHandle() const
{
    return hWnd_;
}

// Updates the height of the ribbon.
void CRibbon::UpdateRibbonHeight()
{
    UINT height = 0;

    if (pFramework_) {
        IUIRibbon *pRibbon = nullptr;
        HRESULT hr = pFramework_->GetView(0, IID_PPV_ARGS(&pRibbon));
        if (SUCCEEDED(hr)) {
            pRibbon->GetHeight(&height);
            pRibbon->Release();
        }
    }

    ribbonHeight_ = static_cast<int>(height);
}

// Retrieves the height of the ribbon.
int CRibbon::RibbonHeight() const
{
    return ribbonHeight_;
}

// Converts QColor to Windows Ribbon framework UI_HSBCOLOR.
UI_HSBCOLOR CRibbon::ColorToHSB(const QColor& color)
{
    double HF, SF, LF;
    color.getHslF(&HF, &SF, &LF);
    double BF = (257.7 + 149.9 * std::log(LF)) / 255.0;
    BF = (BF < 0) ? 0 : (BF > 1) ? 1 : BF;

    BYTE H = static_cast<BYTE>(std::lround(HF * 255.0));
    BYTE S = static_cast<BYTE>(std::lround(SF * 255.0));
    BYTE B = static_cast<BYTE>(std::lround(BF * 255.0));

    return UI_HSB(H, S, B);
}

// Converts Windows Ribbon framework UI_HSBCOLOR to QColor.
QColor CRibbon::HSBToColor(UI_HSBCOLOR hsb)
{
    BYTE H = hsb & 0xFF;
    BYTE S = (hsb >> 8) & 0xFF;
    BYTE B = (hsb >> 16) & 0xFF;

    double HF = H / 255.0;
    double SF = S / 255.0;
    double LF = std::exp((B - 257.7) / 149.9);

    return QColor::fromHslF(HF, SF, LF);
}

// Updates global color properties using native HSB color.
bool CRibbon::SetHSBColor(QPalette::ColorRole role, UI_HSBCOLOR hsb)
{
    if (!pFramework_)
        return false;

    IPropertyStore *store = nullptr;
    if (FAILED(pFramework_->QueryInterface(__uuidof(IPropertyStore), (void**)&store)))
        return false;

    auto updateProperty = [&](const TUIPROPERTYKEY<VT_UI4>& key) {
        PROPVARIANT var;
        ::UIInitPropertyFromUInt32(key, hsb, &var);
        return SUCCEEDED(store->SetValue(key, var)) && SUCCEEDED(store->Commit());
    };

    bool rc = false;
    switch (role) {
    case QPalette::Window:
        rc = updateProperty(UI_PKEY_GlobalBackgroundColor);
        break;
    case QPalette::Highlight:
        rc = updateProperty(UI_PKEY_GlobalHighlightColor);
        break;
    case QPalette::WindowText:
        rc = updateProperty(UI_PKEY_GlobalTextColor);
        break;
    default:
        break;
    }

    store->Release();
    return rc;
}

// Sets a general accent color for the Ribbon.
bool CRibbon::SetAccentColor(const QColor& color)
{
    BYTE h = static_cast<BYTE>(color.hue());
    UI_HSBCOLOR hsb = UI_HSB(h, 127, 245);
    return SetHSBColor(QPalette::Window, hsb);
}

// Retrieves global color properties using a QPalette.
QPalette CRibbon::Palette() const
{
    QPalette pal;
    if (!pFramework_)
        return pal;

    IPropertyStore *store = nullptr;
    if (FAILED(pFramework_->QueryInterface(__uuidof(IPropertyStore), (void**)&store)))
        return pal;

    auto queryPalette = [&](const TUIPROPERTYKEY<VT_UI4>& key, QPalette::ColorRole role) {
        PROPVARIANT var;
        if (SUCCEEDED(store->GetValue(key, &var))) {
            UI_HSBCOLOR hsb = 0;
            ::PropVariantToUInt32(var, &hsb);
            pal.setColor(role, HSBToColor(hsb));
        }
    };

    queryPalette(UI_PKEY_GlobalBackgroundColor, QPalette::Window);
    queryPalette(UI_PKEY_GlobalHighlightColor, QPalette::Highlight);
    queryPalette(UI_PKEY_GlobalTextColor, QPalette::WindowText);

    store->Release();
    return pal;
}

// Updates global color properties using a QPalette.
bool CRibbon::SetPalette(const QPalette& palette)
{
    if (!pFramework_)
        return false;

    IPropertyStore *store = nullptr;
    if (FAILED(pFramework_->QueryInterface(__uuidof(IPropertyStore), (void**)&store)))
        return false;

    auto updatePalette = [&](const TUIPROPERTYKEY<VT_UI4>& key, QPalette::ColorRole role) {
        const QColor& color = palette.color(role);
        if (!color.isValid())
            return false;
        if (color.spec() != QColor::Hsl)
            qCWarning(lcCRibbon, "Incompatible color spec for %s", UIPropertyKeyStr(key));
        UI_HSBCOLOR hsb = ColorToHSB(color);
        PROPVARIANT var;
        ::UIInitPropertyFromUInt32(key, hsb, &var);
        return SUCCEEDED(store->SetValue(key, var));
    };

    bool rc = false;
    if (updatePalette(UI_PKEY_GlobalBackgroundColor, QPalette::Window) ||
        updatePalette(UI_PKEY_GlobalHighlightColor, QPalette::Highlight) ||
        updatePalette(UI_PKEY_GlobalTextColor, QPalette::WindowText))
        rc = SUCCEEDED(store->Commit());

    store->Release();
    return rc;
}

// Retrieves the enabled state for a control.
bool CRibbon::CommandEnabled(UINT32 commandId) const
{
    PROPVARIANT pv;
    if (GetUICommandProperty(commandId, UI_PKEY_Enabled, &pv))
        return UIPropertyValue(UI_PKEY_Enabled, pv);
    return false;
}

// Updates the enabled state for a control.
void CRibbon::SetCommandEnabled(UINT32 commandId, bool enabled)
{
    PROPVARIANT pv;
    ::UIInitPropertyFromBoolean(UI_PKEY_Enabled, enabled, &pv);
    SetUICommandProperty(commandId, UI_PKEY_Enabled, pv);
}

// Retrieves the check state for a CheckBox, ToggleButton, or button in a SplitButtonGallery.
bool CRibbon::CommandChecked(UINT32 commandId) const
{
    PROPVARIANT pv;
    if (GetUICommandProperty(commandId, UI_PKEY_BooleanValue, &pv))
        return UIPropertyValue(UI_PKEY_BooleanValue, pv);
    return false;
}

// Updates the check state for a CheckBox, ToggleButton, or button in a SplitButtonGallery.
void CRibbon::SetCommandChecked(UINT32 commandId, bool checked)
{
    PROPVARIANT pv;
    ::UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, checked, &pv);
    SetUICommandProperty(commandId, UI_PKEY_BooleanValue, pv);
}

// Retrieves the visibility and state of contextual tabs.
UI_CONTEXTAVAILABILITY CRibbon::TabGroupContextAvailability(UINT32 commandId)
{
    PROPVARIANT pv;
    if (GetUICommandProperty(commandId, UI_PKEY_ContextAvailable, &pv))
        return static_cast<UI_CONTEXTAVAILABILITY>(UIPropertyValue(UI_PKEY_ContextAvailable, pv));
    return UI_CONTEXTAVAILABILITY_NOTAVAILABLE;
}

// Updates the visibility and state of contextual tabs.
void CRibbon::SetTabGroupContextAvailability(UINT32 commandId, UI_CONTEXTAVAILABILITY state)
{
    PROPVARIANT pv;
    ::UIInitPropertyFromUInt32(UI_PKEY_ContextAvailable, state, &pv);
    SetUICommandProperty(commandId, UI_PKEY_ContextAvailable, pv);
    InvalidateUICommand(commandId, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_ContextAvailable);
}

UI_SWATCHCOLORTYPE CRibbon::DDCPColorType(UINT32 commandId) const
{
    PROPVARIANT pv;
    if (GetUICommandProperty(commandId, UI_PKEY_ColorType, &pv)) {
        UINT ct; // UI_SWATCHCOLORTYPE
        if (SUCCEEDED(UIPropertyToUInt32(UI_PKEY_ColorType, pv, &ct))) {
            return static_cast<UI_SWATCHCOLORTYPE>(ct);
        }
    }
    return UI_SWATCHCOLORTYPE_NOCOLOR;
}

void CRibbon::SetDDCPColorType(UINT32 commandId, UI_SWATCHCOLORTYPE colorType)
{
    PROPVARIANT pv;
    ::UIInitPropertyFromUInt32(UI_PKEY_ColorType, colorType, &pv);
    SetUICommandProperty(commandId, UI_PKEY_ColorType, pv);
}

QColor CRibbon::DDCPColor(UINT32 commandId) const
{
    UI_SWATCHCOLORTYPE ct = DDCPColorType(commandId);

    switch (ct) {
    case UI_SWATCHCOLORTYPE_RGB:
        PROPVARIANT pv;
        if (GetUICommandProperty(commandId, UI_PKEY_Color, &pv)) {
            UINT rgb; // COLORREF
            if (SUCCEEDED(UIPropertyToUInt32(UI_PKEY_Color, pv, &rgb))) {
                return QColor(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
            }
        }
        break;
    case UI_SWATCHCOLORTYPE_NOCOLOR:
        return QColor(); // QColor::Invalid
    case UI_SWATCHCOLORTYPE_AUTOMATIC:
        return QColor(Qt::black);
    }

    return QColor(); // QColor::Invalid
}

void CRibbon::SetDDCPColor(UINT32 commandId, const QColor& color)
{
    if (!color.isValid()) {
        SetDDCPColorType(commandId, UI_SWATCHCOLORTYPE_NOCOLOR);
        return;
    }

    SetDDCPColorType(commandId, UI_SWATCHCOLORTYPE_RGB);
    COLORREF cr = RGB(color.red(), color.green(), color.blue());
    PROPVARIANT pv;
    ::UIInitPropertyFromUInt32(UI_PKEY_Color, cr, &pv);
    SetUICommandProperty(commandId, UI_PKEY_Color, pv);
}

ULONG __stdcall CRibbon::AddRef()
{
    return ::InterlockedIncrement(&cRef_);
}

ULONG __stdcall CRibbon::Release()
{
    LONG cRef = ::InterlockedDecrement(&cRef_);
    if (cRef == 0)
        delete this;
    return cRef;
}

HRESULT __stdcall CRibbon::QueryInterface(REFIID iid, void **ppv)
{
    if (!ppv)
        return E_INVALIDARG;

    if (iid == __uuidof(IUnknown)) {
        *ppv = static_cast<IUnknown*>(static_cast<IUIApplication*>(this));
    }
    else if (iid == __uuidof(IUICommandHandler)) {
        *ppv = static_cast<IUICommandHandler*>(this);
    }
    else if (iid == __uuidof(IUIApplication)) {
        *ppv = static_cast<IUIApplication*>(this);
    }
    else {
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

// Specifies the application modes to enable.
bool CRibbon::SetModes(UINT modes)
{
    return (pFramework_ && SUCCEEDED(pFramework_->SetModes(modes)));
}

// Processes all pending Command updates.
bool CRibbon::FlushPendingInvalidations()
{
    return (pFramework_ && SUCCEEDED(pFramework_->FlushPendingInvalidations()));
}

// Retrieves a command property, value, or state.
bool CRibbon::GetUICommandProperty(UINT32 commandId, const PROPERTYKEY& key, PROPVARIANT *value) const
{
    return (pFramework_ && SUCCEEDED(pFramework_->GetUICommandProperty(commandId, key, value)));
}

// Sets a command property, value, or state.
bool CRibbon::SetUICommandProperty(UINT32 commandId, const PROPERTYKEY& key, const PROPVARIANT& value)
{
    return (pFramework_ && SUCCEEDED(pFramework_->SetUICommandProperty(commandId, key, value)));
}

// Invalidates a Windows Ribbon framework Command property, value, or state.
bool CRibbon::InvalidateUICommand(UINT32 commandId, UI_INVALIDATIONS flags, const PROPERTYKEY *key)
{
    return (pFramework_ && SUCCEEDED(pFramework_->InvalidateUICommand(commandId, flags, key)));
}

// Called for each Command specified in the Windows Ribbon framework markup to bind the Command to an IUICommandHandler.
HRESULT __stdcall CRibbon::OnCreateUICommand(UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler **ppCommandHandler)
{
    qCDebug(lcCRibbon, "OnCreateUICommand: %d, %s", commandId, UICommandTypeStr(typeId));

    // Contextual tabs are initially disabled.
    if (typeId == UI_COMMANDTYPE_CONTEXT) {
        SetTabGroupContextAvailability(commandId, UI_CONTEXTAVAILABILITY_NOTAVAILABLE);
    }

    return QueryInterface(IID_PPV_ARGS(ppCommandHandler));
}

// Called for each Command specified in the Windows Ribbon framework markup when the application window is destroyed.
HRESULT __stdcall CRibbon::OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeId, IUICommandHandler *pCommandHandler)
{
    UNREFERENCED_PARAMETER(pCommandHandler);

    qCDebug(lcCRibbon, "OnDestroyUICommand: %d, %s", commandId, UICommandTypeStr(typeId));

    return S_OK;
}

// Called when the state of a View changes.
HRESULT __stdcall CRibbon::OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32 reasonCode)
{
    UNREFERENCED_PARAMETER(pView);
    UNREFERENCED_PARAMETER(reasonCode);

    qCDebug(lcCRibbon, "OnViewChanged: %d, %s", viewId, UIViewVerbStr(verb));

    if (typeId == UI_VIEWTYPE_RIBBON) {
        switch (verb)
        {
        case UI_VIEWVERB_CREATE:  // The view was newly created
            UpdateRibbonHeight();
            return S_OK;
        case UI_VIEWVERB_SIZE:    // Ribbon size has changed
            UpdateRibbonHeight();
            UpdateFrameGeometry();
            return S_OK;
        case UI_VIEWVERB_DESTROY: // The view was destroyed
            return S_OK;
        case UI_VIEWVERB_ERROR:   // Action could not be completed
            break;
        }
    }

    return E_NOTIMPL;
}

// Responds to property update requests from the Windows Ribbon framework.
HRESULT __stdcall CRibbon::UpdateProperty(UINT32 commandId, const PROPERTYKEY& key, const PROPVARIANT *currentValue, PROPVARIANT *newValue)
{
    UNREFERENCED_PARAMETER(currentValue);

    qCDebug(lcCRibbon, "UpdateProperty: %d, %s", commandId, UIPropertyKeyStr(key));

    // Set UI_PKEY_Enabled and UI_PKEY_BooleanValue according to UI_CONTEXTAVAILABILITY.
    if (key == UI_PKEY_ContextAvailable) {
        UI_CONTEXTAVAILABILITY state;
        if (newValue && SUCCEEDED(PropVariantToInt32(*newValue, reinterpret_cast<int *>(&state)))) {
            SetCommandEnabled(commandId, state != UI_CONTEXTAVAILABILITY_NOTAVAILABLE);
            SetCommandChecked(commandId, state == UI_CONTEXTAVAILABILITY_ACTIVE);
            return S_OK;
        }
    }

    return E_NOTIMPL;
}

// Responds to execute events on commands bound to the command handler.
HRESULT __stdcall CRibbon::Execute(UINT32 commandId, UI_EXECUTIONVERB verb, const PROPERTYKEY *key, const PROPVARIANT *value, IUISimplePropertySet *executionProperties)
{
    UNREFERENCED_PARAMETER(value);
    UNREFERENCED_PARAMETER(executionProperties);

    qCDebug(lcCRibbon, "Execute: %d, %s, %s", commandId, UIExecutionVerbStr(verb), UIPropertyKeyStr(*key));

    if (!frame_)
        return E_NOTIMPL;

    unsigned long index = UIPropertyKeyIndex(key);

    // Send WM_NOTIFY message on stack to the client frame.
    CRibbonNotify nm = { 0 };
    nm.hdr.hwndFrom = hWnd_;
    nm.hdr.idFrom = commandId;
    nm.hdr.code = verb;
    nm.prop = static_cast<CRibbonProperty>(index);

    ::SendMessage(hWndFrame_, WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>(&nm));

    return S_OK;
}

void CRibbon::EraseRibbonBackground()
{
    static const int borderWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) +
                                   ::GetSystemMetrics(SM_CXPADDEDBORDER);
    static const int captionHeight = ::GetSystemMetrics(SM_CYCAPTION);
    static HBRUSH hbrWhite = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));

    HDC hdc = ::GetWindowDC(hWnd_);
    RECT rr;
    ::GetClientRect(hWnd_, &rr);
    rr.top = captionHeight;
    rr.bottom = RibbonHeight();
    rr.left -= borderWidth;
    rr.right += borderWidth;
    ::FillRect(hdc, &rr, hbrWhite);
}

// Update frame geometry to fit the client rect.
void CRibbon::UpdateFrameGeometry()
{
    RECT cr;
    ::GetClientRect(hWnd_, &cr);
    cr.top = RibbonHeight();
    int x = cr.left / pixelRatio_;
    int y = (cr.top / pixelRatio_);
    int w = (cr.right / pixelRatio_) - x;
    int h = (cr.bottom / pixelRatio_) - y;
    frame_->setGeometry(x, y, w, h);
}

LRESULT __stdcall CRibbon::WndProc(HWND hWnd, UINT32 message, WPARAM wParam, LPARAM lParam)
{
    CRibbon *ribbon = reinterpret_cast<CRibbon*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!ribbon)
        return ::DefWindowProc(hWnd, message, wParam, lParam);

    static const int xMargin = ::GetSystemMetrics(SM_CXSIZEFRAME) +
                               ::GetSystemMetrics(SM_CXPADDEDBORDER);
    static const int yMargin = ::GetSystemMetrics(SM_CYSIZEFRAME);

    LPMINMAXINFO mmi;
    LPWINDOWPOS wp;
    RECT cr;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_DPICHANGED:
        ribbon->pixelRatio_ = ribbon->frame_->devicePixelRatio();
        break;
    case WM_GETMINMAXINFO:
        mmi = reinterpret_cast<LPMINMAXINFO>(lParam);
        mmi->ptMinTrackSize.x = ribbon->frame_->minimumWidth() * ribbon->pixelRatio_ + xMargin * 2;
        mmi->ptMinTrackSize.y = ribbon->frame_->minimumHeight() * ribbon->pixelRatio_ + ribbon->RibbonHeight() + yMargin * 2;
        return 0;
    case WM_WINDOWPOSCHANGING:
        wp = reinterpret_cast<LPWINDOWPOS>(lParam);
        wp->flags |= SWP_NOCOPYBITS;
        return 0;
    case WM_SIZE:
        // Invalidate the client rect below the ribbon.
        ::GetClientRect(hWnd, &cr);
        cr.top = ribbon->RibbonHeight();
        ::InvalidateRect(hWnd, &cr, TRUE);
        return 0;
    case WM_PAINT:
        ::BeginPaint(hWnd, &ps);
        ribbon->UpdateFrameGeometry();
        ::EndPaint(hWnd, &ps);
        return 0;
    case WM_CLOSE:
        if (ribbon->frame_->close()) {
            ribbon->pFramework_->Destroy();
            ribbon->pFramework_->Release();
            QCoreApplication::quit();
        }
        return 0;
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
