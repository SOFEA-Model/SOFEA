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
#include <QString>
#include <QStringList>
#include <QItemSelection>
#include <QStyle>

#include <limits>
#include <vector>

QT_BEGIN_NAMESPACE
class QComboBox;
class QKeyEvent;
class QListWidget;
class QToolButton;
QT_END_NAMESPACE

class ListEditor : public QWidget
{
    Q_OBJECT
    
public:
    ListEditor(QWidget *parent = nullptr);

    void addValue(double p);
    void setValues(std::vector<double>& values);
    void clearValues();
    void setEditable(bool editable);
    void setComboBoxItems(const QStringList& items);
    void setValidator(double min, double max, int decimals);
    void setWhatsThis(const QString& text);
    std::vector<double> values() const;

public slots:
    void onAddClicked();
    void onRemoveClicked();
    void onCurrentIndexChanged(int);
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    
private:
    QComboBox *cboEditor;
    QListWidget *lwEditor;
    QToolButton *btnAdd;
    QToolButton *btnRemove;
    double minValue = std::numeric_limits<double>::lowest();
    double maxValue = std::numeric_limits<double>::max();
};
