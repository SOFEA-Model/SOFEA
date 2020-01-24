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

#include "StatusLabel.h"
#include "AppStyle.h"

#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QWindow>
#include <QtMath>

#include <QDebug>

StatusLabel::StatusLabel(QWidget *parent)
    : QWidget(parent)
{
    m_iconLabel = new QLabel;
    m_iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_iconLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_textLabel = new QLabel;
    m_textLabel->setWordWrap(true);

    QFontMetrics fm = m_textLabel->fontMetrics();

    // Align the icon with the first line of text. Pixmap size is same as ascent.
    int offset = qFloor((fm.lineSpacing() - fm.ascent()) / 2.); // lineSpacing = leading + height
    m_iconLabel->setMinimumSize(fm.ascent(), fm.lineSpacing());
    m_iconLabel->setContentsMargins(0, 0, 0, offset);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignTop);
    setLayout(mainLayout);
}

void StatusLabel::setStatusType(StatusType type)
{
    static QIcon icon;
    switch (type) {
    case StatusLabel::Help:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusHelp));
        break;
    case StatusLabel::OK:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusOK));
        break;
    case StatusLabel::InfoTip:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInfoTip));
        break;
    case StatusLabel::Required:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusRequired));
        break;
    case StatusLabel::Debug:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusDebug));
        break;
    case StatusLabel::Information:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInformation));
        break;
    case StatusLabel::Alert:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusAlert));
        break;
    case StatusLabel::Warning:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusWarning));
        break;
    case StatusLabel::Invalid:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInvalid));
        break;
    case StatusLabel::Critical:
        icon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusCritical));
        break;
    }

    QWindow *window = nullptr;
    if (const QWidget *nativeParent = nativeParentWidget())
        window = nativeParent->windowHandle();

    QFontMetrics fm = m_textLabel->fontMetrics();
    const QPixmap pixmap = icon.pixmap(window, QSize{fm.ascent(), fm.ascent()});
    m_iconLabel->setPixmap(pixmap);
}

void StatusLabel::setText(const QString& text)
{
    m_textLabel->setText(text);
}
