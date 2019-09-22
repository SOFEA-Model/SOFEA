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
