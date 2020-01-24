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

#include "RibbonWindow.h"
#include "CRibbon.h"

#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QFocusEvent>
#include <QStatusBar>

RibbonWindow::RibbonWindow()
{
    statusBar()->setSizeGripEnabled(false);

    if (!QGuiApplication::testAttribute(Qt::AA_EnableHighDpiScaling))
        setAttribute(Qt::WA_StaticContents); // Disable full repaint on resize.

    connect(QGuiApplication::instance(), SIGNAL(focusWindowChanged(QWindow*)),
            this, SLOT(focusWindowChanged(QWindow*)));
}

void RibbonWindow::focusWindowChanged(QWindow *focusWindow)
{
    prevFocus_ = focusWindow;
    if (focusWindow == windowHandle()) {
       QWidget *widget = QApplication::focusWidget();
       if (widget)
           widget->clearFocus();
    }
}

void RibbonWindow::onPolish()
{
    // CRibbon will send WM_NOTIFY messages and update widget geometry.
    ribbon_ =  new CRibbon(this);
    hWndRibbon_ = ribbon_->WindowHandle();

    // Ensure the QMainWindow knows it has a native parent window, as in QWinWidget.
    setProperty("_q_embedded_native_parent_handle", WId(hWndRibbon_));

    // Make the QMainWindow a child of the native host window.
    // This exact order must be followed for proper initialization.
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    ::SetParent((HWND)winId(), hWndRibbon_);
    ::SetWindowLongPtr((HWND)winId(), GWL_STYLE, WS_CHILD);

    // Update the background color for the QMainWindow window class.
    ::SetClassLongPtr((HWND)winId(), GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));

    QEvent e(QEvent::EmbeddingControl);
    QApplication::sendEvent(this, &e);
}

void RibbonWindow::onRibbonNotify(const CRibbonNotify *nm)
{
    const int commandId = static_cast<int>(nm->hdr.idFrom);

    switch (nm->hdr.code) {
    case UI_EXECUTIONVERB_EXECUTE:
        switch (nm->prop) {
        case CRibbonProperty::BooleanValue:
            emit commandToggled(commandId, ribbon_->CommandChecked(commandId));
            break;
        case CRibbonProperty::ColorType:
            emit commandColorChanged(commandId);
            break;
        default:
            emit commandActivated(commandId);
            break;
        }
        break;

    case UI_EXECUTIONVERB_PREVIEW:
        emit commandPreview(commandId);
        break;

    case UI_EXECUTIONVERB_CANCELPREVIEW:
        emit commandCancelPreview(commandId);
        break;
    }
}

bool RibbonWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Polish:
        onPolish();
        break;
    case QEvent::WindowDeactivate:
        prevFocusWidget_ = QApplication::focusWidget();
        break;
    case QEvent::WindowActivate:
        if (prevFocusWidget_)
            prevFocusWidget_->setFocus();
        break;
    default:
        break;
    }

    return QMainWindow::event(e);
}

void RibbonWindow::showEvent(QShowEvent *e)
{
    windowHandle()->show();

    return QMainWindow::showEvent(e);
}

void RibbonWindow::focusInEvent(QFocusEvent *e)
{
    nextInFocusChain()->setFocus();
    return QMainWindow::focusInEvent(e);
}

bool RibbonWindow::nativeEvent(const QByteArray &, void *message, long *result)
{
    Q_UNUSED(result)

    MSG *msg = (MSG *)message;

    switch (msg->message)
    {
    case WM_SETFOCUS:
    {
        Qt::FocusReason reason;
        if (::GetKeyState(VK_LBUTTON) < 0 || ::GetKeyState(VK_RBUTTON) < 0)
            reason = Qt::MouseFocusReason;
        else if (::GetKeyState(VK_SHIFT) < 0)
            reason = Qt::BacktabFocusReason;
        else
            reason = Qt::TabFocusReason;
        QFocusEvent e(QEvent::FocusIn, reason);
        QApplication::sendEvent(this, &e);
        break;
    }
    case WM_NOTIFY:
    {
        // Ribbon messages are handled here.
        CRibbonNotify *nm = reinterpret_cast<CRibbonNotify *>(msg->lParam);
        if (hWndRibbon_ && nm->hdr.hwndFrom == hWndRibbon_)
            onRibbonNotify(nm);
        break;
    }
    default:
        break;
    }

    return false;
}

QWindow* RibbonWindow::windowHandle() const
{
    return QWindow::fromWinId(reinterpret_cast<WId>(hWndRibbon_));
}

QString	RibbonWindow::windowTitle() const
{
    QWindow *w = windowHandle();
    if (w)
        return w->title();
    return QString();
}

void RibbonWindow::setWindowTitle(const QString& title)
{
    QWindow *w = windowHandle();
    if (w)
        w->setTitle(title);
}

void RibbonWindow::setRibbonApplicationModes(unsigned int modes)
{
    if (!ribbon_)
        return;
    ribbon_->SetModes(modes);
}

QPalette RibbonWindow::ribbonPalette() const
{
    if (ribbon_)
        return ribbon_->Palette();
    return QPalette();
}

void RibbonWindow::setRibbonPalette(const QPalette& palette)
{
    if (ribbon_)
        ribbon_->SetPalette(palette);
}

void RibbonWindow::setRibbonAccentColor(const QColor& color)
{
    if (!ribbon_)
        return;
    ribbon_->SetAccentColor(color);
}

bool RibbonWindow::commandEnabled(int commandId) const
{
    if (!ribbon_)
        return false;
    return ribbon_->CommandEnabled(commandId);
}

void RibbonWindow::setCommandEnabled(int commandId, bool enabled)
{
    if (!ribbon_)
        return;
    ribbon_->SetCommandEnabled(commandId, enabled);
}

bool RibbonWindow::commandChecked(int commandId) const
{
    if (!ribbon_)
        return false;
    return ribbon_->CommandChecked(commandId);
}

void RibbonWindow::setCommandChecked(int commandId, bool checked)
{
    if (!ribbon_)
        return;
    ribbon_->SetCommandChecked(commandId, checked);
}

QColor RibbonWindow::commandColor(int commandId) const
{
    if (!ribbon_)
        return QColor();
    return ribbon_->DDCPColor(commandId);
}

void RibbonWindow::setCommandColor(int commandId, const QColor& color)
{
    if (!ribbon_)
        return;
    ribbon_->SetDDCPColor(commandId, color);
}

void RibbonWindow::setTabGroupContextAvailability(int commandId, RibbonWindow::ContextAvailability state)
{
    if (!ribbon_)
        return;
    ribbon_->SetTabGroupContextAvailability(commandId, static_cast<UI_CONTEXTAVAILABILITY>(state));
}

RibbonWindow::ContextAvailability RibbonWindow::tabGroupContextAvailability(int commandId) const
{
    if (!ribbon_)
        return ContextAvailability::NotAvailable;
    return static_cast<RibbonWindow::ContextAvailability>(ribbon_->TabGroupContextAvailability(commandId));
}
