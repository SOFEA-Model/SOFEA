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

#include <QProgressBar>
#include <QPalette>

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
#ifdef Q_OS_WIN
class QWinTaskbarProgress;
#endif
QT_END_NAMESPACE

class ProgressBar : public QProgressBar
{
    Q_OBJECT

    Q_PROPERTY(ProgressState state READ state WRITE setState NOTIFY stateChanged USER true)
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide USER true)

public:
    // Based on Win32 TBPFLAG
    enum ProgressState {
        NoProgress    = 0x00,
        Indeterminate = 0x01,
        Running       = 0x02,
        Error         = 0x04,
        Paused        = 0x08,
    };

    explicit ProgressBar(QWidget *parent = nullptr);

    void setText(const QString& text);

    void setAutoHide(bool);
    bool autoHide();

    void setState(ProgressState state);
    ProgressState state();

#ifdef Q_OS_WIN
    void setTaskbarProgress(QWinTaskbarProgress *);
#endif

signals:
    void stateChanged(ProgressState state);

private slots:
    void onValueChanged(int);

private:
    QPalette defaultPalette_;
    ProgressState state_ = NoProgress;
    bool autoHide_ = false;
};
