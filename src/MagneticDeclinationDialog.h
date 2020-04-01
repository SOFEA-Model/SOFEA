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

#include <QBuffer>
#include <QDialog>
#include <QUrl>

class StatusLabel;
class DoubleLineEdit;
class CurlEasy;

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QWidget;
QT_END_NAMESPACE

class MagneticDeclinationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MagneticDeclinationDialog(QWidget *parent = nullptr);
    void setLongitude(double longitude);
    void setLatitude(double latitude);

signals:
    void declinationUpdated(double value);

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onStartStopClicked();
    void onTransferProgress(qint64 downloadTotal, qint64 downloadNow, qint64 uploadTotal, qint64 uploadNow);
    void onTransferDone();
    void onTransferAborted();

private:
    enum Model {
        WMM,
        IGRF,
        EMM
    };

    QUrl createUrl() const;

    StatusLabel *infoLabel;
    DoubleLineEdit *sbLongitude;
    DoubleLineEdit *sbLatitude;
    QDateEdit *deModelDate;
    QButtonGroup *bgModel;
    QLineEdit *leResult;
    QPushButton *btnStartStop;
    QLabel *statusLabel;
    QDialogButtonBox *buttonBox;

    CurlEasy *transfer = nullptr;
    QBuffer response;

    bool updateComplete = false;
    double declination = 0;
    double uncertainty = 0;
};
