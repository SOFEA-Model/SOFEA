#pragma once

#include <QDialog>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QUrl>

class StatusLabel;

QT_BEGIN_NAMESPACE
class QAuthenticator;
class QButtonGroup;
class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QNetworkReply;
class QPushButton;
class QRadioButton;
class QSslError;
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
    void startRequest(const QUrl& url);
    void abortRequest();
    void httpProgress(qint64 bytesRead, qint64 totalBytes);
    void httpFinished();
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);

private:
    enum Model {
        WMM,
        IGRF,
        EMM
    };

    void setupConnections();
    QUrl createUrl() const;

    StatusLabel *infoLabel;
    QDoubleSpinBox *sbLongitude;
    QDoubleSpinBox *sbLatitude;
    QDateEdit *deModelDate;
    QButtonGroup *bgModel;
    QLineEdit *leResult;
    QPushButton *btnUpdate;
    QLabel *statusLabel;
    QDialogButtonBox *buttonBox;

    QTimer timer;
    QUrl currentUrl;
    QNetworkAccessManager qnam;
    QNetworkReply *reply = nullptr;

    bool updateComplete = false;
    bool httpRequestAborted = false;
    double declination = 0;
    double uncertainty = 0;
};
