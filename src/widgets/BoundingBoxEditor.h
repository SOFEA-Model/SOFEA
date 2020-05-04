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

#include "core/Projection.h"

#include <QWidget>
#include <QRectF>

class DoubleLineEdit;

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QLabel;
QT_END_NAMESPACE

class BoundingBoxEditor : public QWidget
{
    Q_OBJECT

public:
    enum Mode {
        Projected,
        Geographic
    };

    explicit BoundingBoxEditor(QWidget *parent = nullptr);

    void setMode(Mode mode);
    Mode mode() const;

    void setValue(const QRectF& bbox);
    QRectF value() const;

    void setCompoundCRS(std::shared_ptr<PJ> ccrs);
    std::shared_ptr<PJ> compoundCRS() const;

    double xmin() const;
    double ymin() const;
    double xmax() const;
    double ymax() const;

signals:
    void modeChanged(Mode mode);
    void valueChanged(const QRectF& value);

private slots:
    void onXMinChanged(double xmin);
    void onYMinChanged(double ymin);
    void onYMaxChanged(double ymax);
    void onXMaxChanged(double xmax);

private:
    Mode mode_ = Projected;
    std::shared_ptr<PJ> ccrs_;

    QLabel *headerLabel;
    QButtonGroup *optionGroup;
    DoubleLineEdit *xminEdit;
    DoubleLineEdit *yminEdit;
    DoubleLineEdit *ymaxEdit;
    DoubleLineEdit *xmaxEdit;
};
