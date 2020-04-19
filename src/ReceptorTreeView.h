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

#include <QTreeView>
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>

#include "models/ReceptorModel.h"

class ReceptorTreeView : public QTreeView
{
    Q_OBJECT

public:
    ReceptorTreeView(ReceptorModel *model, QWidget *parent = nullptr);
    ~ReceptorTreeView();

    void selectLastRow();
    void copyClipboard();

protected:
    void drawBranches(QPainter *painter, const QRect& rect, const QModelIndex& index) const override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void keyPressEvent(QKeyEvent *event) override;
};
