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

#include "GroupBoxFrame.h"

#include <QPaintEvent>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionGroupBox>
#include <QWidget>

GroupBoxFrame::GroupBoxFrame(QWidget *parent)
    : QFrame(parent)
{
    setAutoFillBackground(true);
}

void GroupBoxFrame::paintEvent(QPaintEvent* event)
{
    QStylePainter p(this);
    QStyleOptionGroupBox opt;
    opt.rect = rect();

    // Remove top margin.
    int indicatorHeight = style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight);
    int topMargin = qMax(indicatorHeight, opt.fontMetrics.height()) + 3; // qfusionstyle.cpp
    opt.rect.adjust(0, -topMargin, 0, 0);

    p.drawPrimitive(QStyle::PE_FrameGroupBox, opt);
}
