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

#ifndef BUFFERZONEEDITOR_H
#define BUFFERZONEEDITOR_H

#include <QWidget>

class BufferZoneModel;
class StandardTableView;
class StatusLabel;

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDoubleSpinBox;
class QItemSelection;
class QLabel;
class QPushButton;
class QSpinBox;
QT_END_NAMESPACE

class BufferZoneEditor : public QWidget
{
    Q_OBJECT

public:
    BufferZoneEditor(BufferZoneModel *m, QWidget *parent = nullptr);
    bool isEnableChecked();
    void setEnableChecked(bool checked);

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onImportClicked();
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);

private:
    BufferZoneModel *model;
    StandardTableView *table;

    QCheckBox *chkEnable;
    QDoubleSpinBox *sbAreaThreshold;
    QDoubleSpinBox *sbAppRateThreshold;
    QPushButton *btnAdd;
    QPushButton *btnRemove;
    QPushButton *btnImport;
    StatusLabel *lblThresholdInfo;
};

#endif // BUFFERZONEEDITOR_H
