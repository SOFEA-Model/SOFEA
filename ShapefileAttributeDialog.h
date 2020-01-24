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

#ifndef SHAPEFILEATTRIBUTEDIALOG_H
#define SHAPEFILEATTRIBUTEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include "ShapefileAttributeInfo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QLabel;
QT_END_NAMESPACE

class ShapefileAttributeDialog : public QDialog
{
    Q_OBJECT

public:
    ShapefileAttributeDialog(const ShapefileAttributeInfo& attrInfo,
                             QMap<ShapefileSelectedIndex, int> *indexMap,
                             QWidget *parent = nullptr);

private slots:
    void onSelectionChanged(int index);

private:
    void init();

    QMap<ShapefileSelectedIndex, int> *pIndexMap;
    QComboBox *cboSourceID;
    QComboBox *cboAppDate;
    QComboBox *cboAppTime;
    QComboBox *cboAppRate;
    QComboBox *cboIncDepth;
};

#endif // SHAPEFILEATTRIBUTEDIALOG_H
