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

#ifndef METEOROLOGYFILEDIALOG_H
#define METEOROLOGYFILEDIALOG_H

#include <QDialog>

class PathEdit;
class StatusLabel;

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE

class MeteorologyFileDialog : public QDialog
{
    Q_OBJECT

public:
    MeteorologyFileDialog(QWidget *parent = nullptr);

private slots:
    void onCurrentPathChanged(const QString& path);
    void accept() override;
    void reject() override;

private:
    QDialogButtonBox *buttonBox;
    PathEdit *leSurfaceFile;
    PathEdit *leUpperAirFile;
};

#endif // METEOROLOGYFILEDIALOG_H
