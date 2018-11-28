#ifndef RECEPTORDIALOG_H
#define RECEPTORDIALOG_H

#include <vector>

#include <QDialog>
#include <QMap>
#include <QStandardItemModel>

#include "StandardPlot.h"
#include "StandardTableView.h"
#include "StandardTableEditor.h"
#include "Utilities.h"

#include "SourceGroup.h"

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QSpinBox;
class QLabel;
class QDialogButtonBox;
QT_END_NAMESPACE

class ReceptorRingTab : public QWidget
{
    Q_OBJECT
public:
    ReceptorRingTab(QStandardItemModel *model, QWidget *parent = nullptr);

private slots:
    void onAddRingClicked();
    void onRemoveRingClicked();

private:
    QDoubleSpinBox *sbRingBuffer;
    QDoubleSpinBox *sbRingSpacing;
    QStandardItemModel *ringModel;
    StandardTableView *ringTable;
    StandardTableEditor *ringEditor;
};

class ReceptorNodeTab : public QWidget
{
    Q_OBJECT
public:
    ReceptorNodeTab(QStandardItemModel *model, QWidget *parent = nullptr);

private slots:
    void onAddNodeClicked();
    void onRemoveNodeClicked();

private:
    DoubleLineEdit *leNodeX;
    DoubleLineEdit *leNodeY;
    DoubleLineEdit *leNodeZ;
    QStandardItemModel *nodeModel;
    StandardTableView *nodeTable;
    StandardTableEditor *nodeEditor;
};

class ReceptorGridTab : public QWidget
{
    Q_OBJECT
public:
    ReceptorGridTab(QStandardItemModel *model, QWidget *parent = nullptr);

private slots:
    void onAddGridClicked();
    void onRemoveGridClicked();

private:
    DoubleLineEdit *leGridXInit;
    DoubleLineEdit *leGridYInit;
    QSpinBox *sbGridXCount;
    QSpinBox *sbGridYCount;
    QDoubleSpinBox *sbGridXDelta;
    QDoubleSpinBox *sbGridYDelta;
    QStandardItemModel *gridModel;
    StandardTableView *gridTable;
    StandardTableEditor *gridEditor;
};

class ReceptorDialog : public QDialog
{
    Q_OBJECT

public:
    ReceptorDialog(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void accept() override;
    void reject() override;
    void updatePlot();

private:
    Scenario *sPtr;
    SourceGroup *sgPtr;
    std::vector<ReceptorRing> ringData() const;
    std::vector<ReceptorNode> nodeData() const;
    std::vector<ReceptorGrid> gridData() const;
    void setRingGeometry(ReceptorRing &ring) const;
    void setGridGeometry(ReceptorGrid &grid) const;

    std::vector<ReceptorRing> rings;
    std::vector<ReceptorNode> nodes;
    std::vector<ReceptorGrid> grids;
    bool saved;

    QStandardItemModel *ringModel;
    QStandardItemModel *nodeModel;
    QStandardItemModel *gridModel;

    ReceptorRingTab *ringTab;
    ReceptorNodeTab *nodeTab;
    ReceptorGridTab *gridTab;

    QLabel *lblReceptorCount;
    StandardPlot *plot;
    QDialogButtonBox *buttonBox;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // RECEPTORDIALOG_H
