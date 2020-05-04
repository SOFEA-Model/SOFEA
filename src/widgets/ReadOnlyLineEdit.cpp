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

#include "ReadOnlyLineEdit.h"

#include <QColor>
#include <QPalette>
#include <QWidget>

ReadOnlyLineEdit::ReadOnlyLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setReadOnly(true);

    QPalette p = QLineEdit::palette();
    QColor textColor = p.color(QPalette::Disabled, QPalette::Text);
    p.setColor(QPalette::Text, textColor);
    setPalette(p);
}
