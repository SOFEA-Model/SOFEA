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

#include "Projection.h"

#include <QWidget>
#include <string>

class DoubleLineEdit;
class ExtendedComboBox;
class PlainTextEdit;
class ZoneModel;

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QSpinBox;
class QStackedWidget;
QT_END_NAMESPACE

class ProjectionEditor : public QWidget
{
    Q_OBJECT

public:
    ProjectionEditor(QWidget *parent = nullptr);    

    void setConversionCode(const std::string& code);
    void setHDatumCode(const std::string& code);
    void setHUnitsCode(const std::string& code);
    void setVDatumCode(const std::string& code);
    void setVUnitsCode(const std::string& code);

    std::string conversionCode() const;
    std::string hDatumCode() const;
    std::string hUnitsCode() const;
    std::string vDatumCode() const;
    std::string vUnitsCode() const;

private slots:
    void onProjectionChanged(int index);

private:
    void setupConnections();
    void updatePreview();

    ZoneModel *zoneModel;
    QComboBox *cboProjection;
    ExtendedComboBox *cboZone;
    QComboBox *cboHDatum;
    QComboBox *cboHUnits;
    QComboBox *cboVDatum;
    QComboBox *cboVUnits;
    PlainTextEdit *textPreview;
};
