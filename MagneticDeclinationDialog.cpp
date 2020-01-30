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

// Widgets
#include <QBoxLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QRadioButton>

// Network
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSslError>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include "AppStyle.h"
#include "widgets/StatusLabel.h"

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

    sbLongitude = new QDoubleSpinBox;
    sbLongitude->setRange(-180.0, 180.0);
    sbLongitude->setDecimals(5);
    sbLongitude->setSuffix(QLatin1String("\x00b0"));

    sbLatitude = new QDoubleSpinBox;
    sbLatitude->setRange(-90.0, 90.0);
    sbLatitude->setDecimals(5);
    sbLatitude->setSuffix(QLatin1String("\x00b0"));

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

    btnUpdate = new QPushButton(tr("Update"));
    btnUpdate->setDefault(true);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

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
    updateLayout->addWidget(btnUpdate, 0, Qt::AlignRight);

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
    setupConnections();
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
    if (updateComplete)
        emit declinationUpdated(declination);

    QDialog::done(QDialog::Accepted);
}

void MagneticDeclinationDialog::reject()
{
    QDialog::done(QDialog::Rejected);
}

void MagneticDeclinationDialog::setupConnections()
{
    timer.setSingleShot(true);

    connect(&timer, &QTimer::timeout,
            this, &MagneticDeclinationDialog::abortRequest);

    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);

    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    connect(btnUpdate, &QPushButton::clicked, [=]() {
        leResult->clear();
        QUrl newUrl = createUrl();
        startRequest(newUrl);
    });

    connect(&qnam, &QNetworkAccessManager::sslErrors,
            this, &MagneticDeclinationDialog::sslErrors);
}

QUrl MagneticDeclinationDialog::createUrl() const
{
    QUrl url;
    QUrlQuery query;

    int model = bgModel->checkedId();
    switch (model) {
    case Model::WMM:
        url.setUrl("https://www.ngdc.noaa.gov/geomag-web/calculators/calculateDeclination?");
        query.addQueryItem("model", "WMM");
        break;
    case Model::IGRF:
        url.setUrl("https://www.ngdc.noaa.gov/geomag-web/calculators/calculateDeclination?");
        query.addQueryItem("model", "IGRF");
        break;
    case Model::EMM:
        url.setUrl("https://emmcalc.geomag.info/?");
        query.addQueryItem("magneticComponent", "d");
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

void MagneticDeclinationDialog::startRequest(const QUrl& url)
{
    currentUrl = url;
    httpRequestAborted = false;
    btnUpdate->setEnabled(false);
    statusLabel->setText(tr("Connecting..."));

    timer.start(15000); // 15 second timeout
    reply = qnam.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished,
            this, &MagneticDeclinationDialog::httpFinished);

    connect(reply, &QNetworkReply::downloadProgress,
            this, &MagneticDeclinationDialog::httpProgress);
}

void MagneticDeclinationDialog::abortRequest()
{
    if (reply == nullptr)
        return;

    if (timer.isActive())
        timer.stop();

    httpRequestAborted = true;
    reply->abort();
    statusLabel->setText(tr("Update canceled"));
    btnUpdate->setEnabled(true);
}

void MagneticDeclinationDialog::httpProgress(qint64 bytesRead, qint64 totalBytes)
{
    statusLabel->setText(tr("Downloading..."));
    return;
}

void MagneticDeclinationDialog::httpFinished()
{
    if (timer.isActive())
        timer.stop();

    if (httpRequestAborted) {
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    if (reply->error()) {
        statusLabel->setText(tr("Network Error: %1").arg(reply->errorString()));
        btnUpdate->setEnabled(true);
        reply->deleteLater();
        reply = nullptr;
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull()) {
        reply->deleteLater();
        reply = nullptr;
        const QUrl redirectedUrl = currentUrl.resolved(redirectionTarget.toUrl());
        startRequest(redirectedUrl);
        return;
    }

    // Decode the response.
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    reply = nullptr;

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

    if (updateComplete) {
        statusLabel->setText(tr("Update complete"));
    }
    else {
        statusLabel->setText(tr("Error: Invalid data received."));
    }

    btnUpdate->setEnabled(true);
}

void MagneticDeclinationDialog::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    QString message = tr("One or more SSL errors have occurred:\n%1").arg(errorString);
    int rc = QMessageBox::warning(this, tr("SSL Errors"), message, QMessageBox::Ignore | QMessageBox::Abort);
    if (rc == QMessageBox::Ignore)
        reply->ignoreSslErrors();
}
