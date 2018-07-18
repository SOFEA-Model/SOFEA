//#include <QtWidgets>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>

#include "ShapefileAttributeDialog.h"

ShapefileAttributeDialog::ShapefileAttributeDialog(
        const ShapefileAttributeInfo &attrInfo,
        QMap<ShapefileSelectedIndex, int> *indexMap, QWidget *parent)
    : pIndexMap(indexMap), QDialog(parent)
{
    setWindowTitle(tr("Select Attributes"));
    setWindowModality(Qt::ApplicationModal);

    QLabel *lblSourceID = new QLabel("Source ID: ");
    QLabel *lblAppDate = new QLabel("Application start date (yyyy-mm-dd): ");
    QLabel *lblAppTime = new QLabel("Application start time (hhmm): ");
    QLabel *lblAppRate = new QLabel("Application rate (kg/ha): ");
    QLabel *lblIncDepth = new QLabel("Incorporation depth (cm): ");

    cboSourceID = new QComboBox;
    cboAppDate = new QComboBox;
    cboAppTime = new QComboBox;
    cboAppRate = new QComboBox;
    cboIncDepth = new QComboBox;

    for (int i = 0; i < attrInfo.fieldCount; ++i)
    {
        DBFFieldType ftype = attrInfo.fieldInfoMap[i].type;
        QString fname = attrInfo.fieldInfoMap[i].name;

        if (ftype == FTString) {
            cboSourceID->insertItem(0, fname, i);
            cboAppDate->insertItem(0, fname, i);
            cboAppTime->insertItem(0, fname, i);
        }
        else if (ftype == FTInteger || ftype == FTDouble) {
            cboAppRate->insertItem(0, fname, i);
            cboIncDepth->insertItem(0, fname, i);
        }
    }

    // Set default items.
    cboSourceID->insertItem(0, "", -1);
    cboAppDate->insertItem(0, "", -1);
    cboAppTime->insertItem(0, "", -1);
    cboAppRate->insertItem(0, "", -1);
    cboIncDepth->insertItem(0, "", -1);

    cboSourceID->setCurrentIndex(0);
    cboAppDate->setCurrentIndex(0);
    cboAppTime->setCurrentIndex(0);
    cboAppRate->setCurrentIndex(0);
    cboIncDepth->setCurrentIndex(0);

    QFormLayout *controlsLayout = new QFormLayout;
    controlsLayout->addRow(lblSourceID, cboSourceID);
    controlsLayout->addRow(lblAppDate, cboAppDate);
    controlsLayout->addRow(lblAppTime, cboAppTime);
    controlsLayout->addRow(lblAppRate, cboAppRate);
    controlsLayout->addRow(lblIncDepth, cboIncDepth);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(controlsLayout);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(buttonBox);

    init();
    setLayout(mainLayout);
}

void ShapefileAttributeDialog::init()
{
    connect(cboSourceID, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &ShapefileAttributeDialog::onSelectionChanged);

    connect(cboAppDate, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &ShapefileAttributeDialog::onSelectionChanged);

    connect(cboAppTime, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &ShapefileAttributeDialog::onSelectionChanged);

    connect(cboAppRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &ShapefileAttributeDialog::onSelectionChanged);

    connect(cboIncDepth, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &ShapefileAttributeDialog::onSelectionChanged);
}

void ShapefileAttributeDialog::onSelectionChanged(int index)
{
    ShapefileSelectedIndex key;
    int val;

    QComboBox *sender = qobject_cast<QComboBox *>(QObject::sender());
    if (sender == cboSourceID) {
        key = ShapefileSelectedIndex::SourceID;
        val = cboSourceID->currentData().toInt();
    }
    else if (sender == cboAppDate) {
        key = ShapefileSelectedIndex::AppDate;
        val = cboAppDate->currentData().toInt();
    }
    else if (sender == cboAppTime) {
        key = ShapefileSelectedIndex::AppTime;
        val = cboAppTime->currentData().toInt();
    }
    else if (sender == cboAppRate) {
        key = ShapefileSelectedIndex::AppRate;
        val = cboAppRate->currentData().toInt();
    }
    else if (sender == cboIncDepth) {
        key = ShapefileSelectedIndex::IncDepth;
        val = cboIncDepth->currentData().toInt();
    }
    else {
        return;
    }

    pIndexMap->insert(key, val);
}
