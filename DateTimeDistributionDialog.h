#ifndef DATETIMEDISTRIBUTIONDIALOG_H
#define DATETIMEDISTRIBUTIONDIALOG_H

#include <QDialog>

#include "DateTimeDistribution.h"
#include "widgets/StandardTableView.h"
#include "widgets/StatusLabel.h"

QT_BEGIN_NAMESPACE
class QDateTimeEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QStandardItemModel;
QT_END_NAMESPACE

class DateTimeDistributionDialog : public QDialog
{
    Q_OBJECT

public:
    DateTimeDistributionDialog(const DateTimeDistribution &d, QWidget *parent = nullptr);
    DateTimeDistribution getDistribution() const;

private slots:
    void onAddClicked();
    void onClearClicked();

private:
    void init();
    void resetTable();

public slots:
    void apply();
    void accept() override;
    void reject() override;

private:
    IntervalMap intervals;
    DateTimeDistribution currentDist;

    QDateTimeEdit *dteLower;
    QDateTimeEdit *dteUpper;
    QStandardItemModel *model;
    StandardTableView *view;
    QPushButton *btnAdd;
    QPushButton *btnClear;
    StatusLabel *lblStatus;
    QDialogButtonBox *buttonBox;
};

#endif // DATETIMEDISTRIBUTIONDIALOG_H
