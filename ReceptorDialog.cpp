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
#include <QDialogButtonBox>
#include <QIcon>
#include <QKeyEvent>
#include <QShowEvent>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "AppStyle.h"
#include "ReceptorDialog.h"
#include "ReceptorEditor.h"
#include "Scenario.h"

//-----------------------------------------------------------------------------
// ReceptorDialog
//-----------------------------------------------------------------------------

ReceptorDialog::ReceptorDialog(Scenario *s, QWidget *parent)
    : QDialog(parent), sPtr(s)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionEditorZone)));
    setWindowTitle(QString::fromStdString(sPtr->name));

    editor = new ReceptorEditor;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setContentsMargins(16, 16, 16, 16);

    QPushButton *buttonOk = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *buttonCancel = buttonBox->button(QDialogButtonBox::Cancel);
    buttonOk->setAutoDefault(false);
    buttonCancel->setAutoDefault(false);
    buttonCancel->setDefault(true);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(editor, 1);
    mainLayout->addWidget(buttonBox, 0);

    setLayout(mainLayout);

    editor->load(sPtr);
    editor->setFocus();

    connect(buttonBox, &QDialogButtonBox::rejected, this, &ReceptorDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ReceptorDialog::accept);
}

ReceptorDialog::~ReceptorDialog()
{}

void ReceptorDialog::accept()
{
    editor->save(sPtr);
    QDialog::done(QDialog::Accepted);
}

void ReceptorDialog::reject()
{
    QDialog::done(QDialog::Rejected);
}

void ReceptorDialog::keyPressEvent(QKeyEvent *event)
{
    // Prevent accidental close.
    if (event->key() == Qt::Key_Escape)
        return;

    QDialog::keyPressEvent(event);
}

