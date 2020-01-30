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

#include <QApplication>
#include <QBoxLayout>
#include <QKeyEvent>
#include <QPalette>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <QDebug>

#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Navigation Tree
    navTree = new QTreeWidget();
    navTree->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    navTree->setUniformRowHeights(true);
    navTree->setHeaderHidden(true);
    navTree->setColumnCount(1);
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() + 1);
    navTree->setFont(font);

    QPalette navPalette = navTree->viewport()->palette();
    QColor baseColor = QWidget::palette().window().color().lighter(104);
    navPalette.setColor(QPalette::Base, baseColor);
    navTree->viewport()->setAutoFillBackground(true);
    navTree->viewport()->setPalette(navPalette);

    pageStack = new QStackedWidget;

    //helpTextEdit = new ReadOnlyTextEdit;
    //helpTextEdit->setLineCount(4);
    //helpTextEdit->setFocusPolicy(Qt::NoFocus);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);

    // Connections
    connect(navTree->selectionModel(), &QItemSelectionModel::currentChanged,
        [=](const QModelIndex& current, const QModelIndex& previous) {
        pageStack->setCurrentIndex(current.row());
        //helpTextEdit->clear();
    });

    //connect(qApp, &QApplication::focusChanged, this, &SettingsDialog::onFocusChanged);

    installEventFilter(this);

    // Layout
    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(pageStack);
    //verticalLayout->addWidget(helpTextEdit);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(navTree, 0);
    horizontalLayout->addLayout(verticalLayout, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void SettingsDialog::addPage(QString const& label, QWidget *page)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, label);
    navTree->addTopLevelItem(item);
    navTreeItems[item] = page;
    pageStack->addWidget(page);
}

void SettingsDialog::onFocusChanged(const QWidget *, const QWidget *now)
{
    //if (now && !now->whatsThis().isEmpty()) {
    //    QString helpText = now->whatsThis();
    //    helpTextEdit->setText(helpText);
    //}
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
            return true; // Ignore escape key
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}
