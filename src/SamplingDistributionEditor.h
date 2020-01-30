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

#ifndef SAMPLINGDISTRIBUTIONEDITOR_H
#define SAMPLINGDISTRIBUTIONEDITOR_H

#include <QWidget>
#include <QString>

#include "SamplingProxyModel.h"
#include "SamplingDistribution.h"
#include "widgets/StandardTableView.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QStandardItemModel;
QT_END_NAMESPACE

class SamplingDistributionEditor : public QWidget
{
    Q_OBJECT

public:
    SamplingDistributionEditor(QAbstractItemModel *model, QWidget *parent = nullptr);
    void setColumnHidden(int column);
    bool setProbability(int row, double value);
    double getProbability(int row) const;
    void normalize();

private:
    SamplingProxyModel *proxyModel;
    StandardTableView *view;
    QPushButton *btnNormalize;
};

#endif // SAMPLINGDISTRIBUTIONEDITOR_H
