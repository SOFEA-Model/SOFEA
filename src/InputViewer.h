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

#include <QWidget>

#include <cstddef>
#include <string>

struct Scenario;
class ScintillaEditBase;
struct SCNotification;

class InputViewer : public QWidget
{
    Q_OBJECT

public:
    explicit InputViewer(Scenario *s, QWidget *parent = nullptr);
    void setText(const std::string& text);

public slots:
    void refresh();

private:
    intptr_t sendMessage(unsigned int message, uintptr_t wParam = 0, intptr_t lParam = 0);
    void defineMarker(int marker, int markerType, int fg, int bg);
    void setupStyles();
    void setupMargins();
    void setupConnections();
    void updateFoldState();

private slots:
    void onMarginClicked(int position, int modifiers, int margin);
    void onStyleNeeded(int position);
    void onNotify(SCNotification *pscn);
    void onCommand(uintptr_t wParam, intptr_t lParam);

private:
    Scenario *sPtr;
    ScintillaEditBase *sci;
};
