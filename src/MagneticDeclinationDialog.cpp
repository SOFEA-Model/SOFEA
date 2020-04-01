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

#include "MagneticDeclinationDialog.h"

#include "AppStyle.h"
#include "core/Common.h"
#include "qtcurl/CurlEasy.h"
#include "widgets/DoubleLineEdit.h"
#include "widgets/StatusLabel.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>

#include <QDebug>

MagneticDeclinationDialog::MagneticDeclinationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Magnetic Declination Calculator"));
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_CalculateButton)));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = currentDateTime.date();
    int currentYear = currentDate.year();

    QString infoText = tr(
        "This calculator estimates the correction angle to true north "
        "using NCEI geomagnetic models, when wind direction is referenced "
        "to magnetic north. This is uncommon. Ensure that wind direction is "
        "referenced to magnetic north before applying any corrections.");

    infoLabel = new StatusLabel;
    infoLabel->setStatusType(StatusLabel::InfoTip);
    infoLabel->setText(infoText);

    sbLongitude = new DoubleLineEdit(-180.0, 180.0, 6);
    sbLatitude = new DoubleLineEdit(-90.0, 90.0, 6);

    deModelDate = new QDateEdit;
    deModelDate->setDisplayFormat("yyyy-MM-dd");
    deModelDate->setMinimumDate(QDate(1590, 1, 1));
    deModelDate->setMaximumDate(currentDate);
    deModelDate->setDate(currentDate);

    QString textWMM = QString("WMM (2014-%1)").arg(currentYear);
    QString textIGRF = QString("IGRF (1590-%1)").arg(currentYear);
    QString textEMM = QString("EMM (2000-%1)").arg(currentYear);

    QRadioButton *radioWMM = new QRadioButton(textWMM);
    radioWMM->setToolTip("World Magnetic Model");
    radioWMM->setChecked(true);

    QRadioButton *radioIGRF = new QRadioButton(textIGRF);
    radioIGRF->setToolTip("International Geomagnetic Reference Field");

    QRadioButton *radioEMM = new QRadioButton(textEMM);
    radioEMM->setToolTip("Enhanced Magnetic Model");

    bgModel = new QButtonGroup;
    bgModel->addButton(radioWMM, Model::WMM);
    bgModel->addButton(radioIGRF, Model::IGRF);
    bgModel->addButton(radioEMM, Model::EMM);

    leResult = new QLineEdit;
    leResult->setReadOnly(true);

    statusLabel = new QLabel;

    btnStartStop = new QPushButton(tr("Update"));
    btnStartStop->setDefault(true);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    transfer = new CurlEasy(this);

    // Connections
    connect(btnStartStop, &QPushButton::clicked, this, &MagneticDeclinationDialog::onStartStopClicked);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(transfer, &CurlEasy::done, this, &MagneticDeclinationDialog::onTransferDone);
    connect(transfer, &CurlEasy::aborted, this, &MagneticDeclinationDialog::onTransferAborted);
    connect(transfer, &CurlEasy::progress, this, &MagneticDeclinationDialog::onTransferProgress);

    // Input Layout
    QHBoxLayout *modelLayout = new QHBoxLayout;
    modelLayout->setContentsMargins(0, 0, 0, 0);
    modelLayout->setSpacing(10);
    modelLayout->addWidget(radioWMM);
    modelLayout->addWidget(radioIGRF);
    modelLayout->addWidget(radioEMM);
    modelLayout->setAlignment(radioWMM, Qt::AlignLeft);
    modelLayout->setAlignment(radioIGRF, Qt::AlignLeft);
    modelLayout->setAlignment(radioEMM, Qt::AlignLeft);

    QGridLayout *inputLayout = new QGridLayout;
    inputLayout->setColumnMinimumWidth(0, 125);
    inputLayout->setColumnStretch(0, 0);
    inputLayout->setColumnStretch(1, 1);
    inputLayout->addWidget(new QLabel(tr("Longitude:")), 0, 0);
    inputLayout->addWidget(sbLongitude, 0, 1);
    inputLayout->addWidget(new QLabel(tr("Latitude:")), 1, 0);
    inputLayout->addWidget(sbLatitude, 1, 1);
    inputLayout->addWidget(new QLabel(tr("Date:")), 2, 0);
    inputLayout->addWidget(deModelDate, 2, 1);
    inputLayout->addWidget(new QLabel(tr("Model:")), 3, 0);
    inputLayout->addLayout(modelLayout, 3, 1);

    // Output Layout
    QHBoxLayout *updateLayout = new QHBoxLayout;
    updateLayout->addWidget(statusLabel, 1);
    updateLayout->addWidget(btnStartStop, 0, Qt::AlignRight);

    QGridLayout *outputLayout = new QGridLayout;
    outputLayout->setColumnMinimumWidth(0, 125);
    outputLayout->setColumnStretch(0, 0);
    outputLayout->setColumnStretch(1, 1);
    outputLayout->addWidget(new QLabel(tr("Declination:")), 0, 0);
    outputLayout->addWidget(leResult, 0, 1);
    outputLayout->addLayout(updateLayout, 2, 0, 1, 2);

    // Main Layout
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(separator);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(outputLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

QUrl MagneticDeclinationDialog::createUrl() const
{
    QUrl url;
    QUrlQuery query;

    int model = bgModel->checkedId();
    switch (model) {
    case Model::WMM:
        url.setUrl(GEOMAG_API_URL_WMM);
        break;
    case Model::IGRF:
        url.setUrl(GEOMAG_API_URL_IGRF);
        break;
    case Model::EMM:
        url.setUrl(GEOMAG_API_URL_EMM);
        break;
    }

    QDate date = deModelDate->date();
    query.addQueryItem("lat1", QString::number(sbLatitude->value()));
    query.addQueryItem("lon1", QString::number(sbLongitude->value()));
    query.addQueryItem("startYear", QString::number(date.year()));
    query.addQueryItem("startMonth", QString::number(date.month()));
    query.addQueryItem("startDay", QString::number(date.day()));
    query.addQueryItem("resultFormat", "json");
    url.setQuery(query.query());
    return url;
}

void MagneticDeclinationDialog::setLongitude(double longitude)
{
    sbLongitude->setValue(longitude);
}

void MagneticDeclinationDialog::setLatitude(double latitude)
{
    sbLatitude->setValue(latitude);
}

void MagneticDeclinationDialog::accept()
{
    if (transfer->isRunning())
        transfer->abort();

    if (updateComplete)
        emit declinationUpdated(declination);

    QDialog::done(QDialog::Accepted);
}

void MagneticDeclinationDialog::reject()
{
    if (transfer->isRunning())
        transfer->abort();

    QDialog::done(QDialog::Rejected);
}

void MagneticDeclinationDialog::onStartStopClicked()
{
    if (transfer->isRunning()) {
        transfer->abort();
        return;
    }

    leResult->clear();
    btnStartStop->setText(tr("Abort"));
    statusLabel->setText(tr("Connecting..."));

    response.buffer().clear();
    response.open(QIODevice::ReadWrite);

    transfer->setWriteFunction([this](char *data, std::size_t size) {
        qint64 bytesWritten = response.write(data, static_cast<qint64>(size));
        return static_cast<size_t>(bytesWritten);
    });

    transfer->set(CURLOPT_URL, createUrl());
    transfer->set(CURLOPT_FOLLOWLOCATION, long(1));
    transfer->set(CURLOPT_FAILONERROR, long(1));

    transfer->perform();
}

void MagneticDeclinationDialog::onTransferProgress(qint64 downloadTotal, qint64 downloadNow, qint64, qint64)
{
    qint64 progress = 0;
    if (downloadTotal > 0) {
        if (downloadNow > downloadTotal) downloadNow = downloadTotal;
        progress = (downloadNow / downloadTotal) * 100;
    }

    statusLabel->setText(QString("Downloading... %1%").arg(progress));
}

void MagneticDeclinationDialog::onTransferDone()
{
    if (transfer->result() != CURLE_OK) {
        statusLabel->setText(tr("Network Error: %1").arg(curl_easy_strerror(transfer->result())));
        btnStartStop->setText(tr("Update"));
        response.buffer().clear();
        response.close();
        return;
    }

    // Decode the response.
    QJsonDocument document = QJsonDocument::fromJson(response.buffer());
    response.buffer().clear();
    response.close();

    updateComplete = false;

    QJsonObject json = document.object();
    if (json.contains("result") && json["result"].isArray()) {
        QJsonArray result = json["result"].toArray();
        if (result.count() == 1 && result.first().isObject()) {
            QJsonObject inner = result.first().toObject();
            if (inner.contains("declination") && inner["declination"].isDouble()) {
                declination = inner["declination"].toDouble();
                QString resultText;
                resultText.append(QString::number(declination));
                resultText.append(QLatin1String("\x00b0"));
                if (inner.contains("declination_uncertainty") && inner["declination_uncertainty"].isDouble()) {
                    uncertainty = inner["declination_uncertainty"].toDouble();
                    resultText.append(QLatin1String(" \x00b1 "));
                    resultText.append(QString::number(uncertainty));
                    resultText.append(QLatin1String("\x00b0"));
                }
                leResult->setText(resultText);
                updateComplete = true;
            }
        }
    }

    if (updateComplete)
        statusLabel->setText(tr("Update complete"));
    else
        statusLabel->setText(tr("Error: Invalid data received."));

    btnStartStop->setText(tr("Update"));
}

void MagneticDeclinationDialog::onTransferAborted()
{
    response.buffer().clear();
    response.close();
    statusLabel->setText(tr("Update aborted"));
    btnStartStop->setText(tr("Update"));
}
