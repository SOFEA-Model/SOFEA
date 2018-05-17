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

class ReceptorDialog : public QDialog
{
    Q_OBJECT

public:
    ReceptorDialog(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();


private slots:
    void accept() override;
    void reject() override;
    void onAddRingClicked();
    void onRemoveRingClicked();
    void onAddNodeClicked();
    void onRemoveNodeClicked();
    void onAddGridClicked();
    void onRemoveGridClicked();
    void updatePlot();

private:
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

    // Ring Controls
    QDoubleSpinBox *sbRingBuffer;
    QDoubleSpinBox *sbRingSpacing;
    QStandardItemModel *ringModel;
    StandardTableView *ringTable;
    StandardTableEditor *ringEditor;

    // Node Controls
    DoubleLineEdit *leNodeX;
    DoubleLineEdit *leNodeY;
    QStandardItemModel *nodeModel;
    StandardTableView *nodeTable;
    StandardTableEditor *nodeEditor;

    // Grid Controls
    DoubleLineEdit *leGridXInit;
    DoubleLineEdit *leGridYInit;
    QSpinBox *sbGridXCount;
    QSpinBox *sbGridYCount;
    QDoubleSpinBox *sbGridXDelta;
    QDoubleSpinBox *sbGridYDelta;
    QStandardItemModel *gridModel;
    StandardTableView *gridTable;
    StandardTableEditor *gridEditor;

    QLabel *lblReceptorCount;
    StandardPlot *plot;
    QDialogButtonBox *buttonBox;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // RECEPTORDIALOG_H
