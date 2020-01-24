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

#include <QColor>
#include <QEvent>
#include <QMainWindow>
#include <QPalette>
#include <QString>
#include <QWindow>

struct CRibbonNotify;
class CRibbon;

class RibbonWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    RibbonWindow();
    ~RibbonWindow() = default;

    QWindow* windowHandle() const;
    QString windowTitle() const;
    void setWindowTitle(const QString& title);

    void setRibbonApplicationModes(unsigned int mode);
    QPalette ribbonPalette() const;
    void setRibbonPalette(const QPalette& palette);
    void setRibbonAccentColor(const QColor& color);

    bool commandEnabled(int commandId) const;
    void setCommandEnabled(int commandId, bool enabled);
    bool commandChecked(int commandId) const;
    void setCommandChecked(int commandId, bool checked);
    QColor commandColor(int commandId) const;
    void setCommandColor(int commandId, const QColor& color);

    enum ContextAvailability {
        NotAvailable = 0,
        Available = 1,
        Active = 2
    };

    void setTabGroupContextAvailability(int commandId, RibbonWindow::ContextAvailability state);
    RibbonWindow::ContextAvailability tabGroupContextAvailability(int commandId)  const;

signals:
    void commandActivated(int commandId);
    void commandToggled(int commandId, bool checked);
    void commandColorChanged(int commandId);
    void commandPreview(int commandId);
    void commandCancelPreview(int commandId);

private slots:
    void focusWindowChanged(QWindow *focusWindow);

protected:
    bool event(QEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    void onPolish();
    void onRibbonNotify(const CRibbonNotify *nm);

    CRibbon *ribbon_ = nullptr;
    HWND hWndRibbon_ = nullptr;
    QWindow *prevFocus_ = nullptr;
    QWidget *prevFocusWidget_ = nullptr;
};
