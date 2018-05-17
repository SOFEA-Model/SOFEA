#ifndef STANDARDTABLEVIEW_H
#define STANDARDTABLEVIEW_H

#include <vector>

#include <QTableView>
#include <QDateTime>
#include <QKeyEvent>
#include <QVariant>

#include "ItemDelegate.h"

class StandardTableView : public QTableView
{
    Q_OBJECT
public:
    explicit StandardTableView(QWidget *parent = nullptr);
    void addRow();
    void selectLastRow();
    void removeSelectedRows();
    void copyClipboard();
    void setDoubleLineEdit(double min, double max, int decimals, bool fixed = false);
    void setDoubleLineEditForColumn(int column, double min, double max, int decimals, bool fixed = false);
    void setSpinBox(int min, int max);
    void setSpinBoxForColumn(int column, int min, int max);
    void setDoubleSpinBox(double min, double max, int decimals, bool fixed = false);
    void setDoubleSpinBoxForColumn(int column, double min, double max, int decimals, bool fixed = false);
    void setDateTimeEdit(QDateTime min = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0, 0)),
                         QDateTime max = QDateTime(QDate(2100, 12, 31), QTime(23, 59, 59, 999)));
    void setDateTimeEditForColumn(int column,
                                  QDateTime min = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0, 0)),
                                  QDateTime max = QDateTime(QDate(2100, 12, 31), QTime(23, 59, 59, 999)));
    std::vector<double> getNumericColumn(int);
    template <typename T> void setColumnData(int column, const std::vector<T> &values);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // STANDARDTABLEVIEW_H

