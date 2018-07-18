#ifndef SHAPEFILEATTRIBUTEDIALOG_H
#define SHAPEFILEATTRIBUTEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include "ShapefileAttributeInfo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QLabel;
QT_END_NAMESPACE

class ShapefileAttributeDialog : public QDialog
{
    Q_OBJECT

public:
    ShapefileAttributeDialog(const ShapefileAttributeInfo& attrInfo,
                             QMap<ShapefileSelectedIndex, int> *indexMap,
                             QWidget *parent = nullptr);

private slots:
    void onSelectionChanged(int index);

private:
    void init();

    QMap<ShapefileSelectedIndex, int> *pIndexMap;
    QComboBox *cboSourceID;
    QComboBox *cboAppDate;
    QComboBox *cboAppTime;
    QComboBox *cboAppRate;
    QComboBox *cboIncDepth;
};

#endif // SHAPEFILEATTRIBUTEDIALOG_H
