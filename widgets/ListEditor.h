#pragma once

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QItemSelection>

#include <limits>
#include <vector>

QT_BEGIN_NAMESPACE
class QComboBox;
class QListWidget;
class QToolButton;
QT_END_NAMESPACE

class ListEditor : public QWidget
{
    Q_OBJECT
    
public:
    ListEditor(QWidget *parent = nullptr);

    void addValue(double p);
    void setValues(std::vector<double>& values);
    void clearValues();
    void setEditable(bool editable);
    void setComboBoxItems(const QStringList& items);
    void setValidator(double min, double max, int decimals);
    void setWhatsThis(const QString& text);
    void resetLayout();
    std::vector<double> values() const;

public slots:
    void onAddClicked();
    void onRemoveClicked();
    void onCurrentIndexChanged(int);
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
    
private:
    QComboBox *cboEditor;
    QListWidget *lwEditor;
    QToolButton *btnAdd;
    QToolButton *btnRemove;
    double minValue = std::numeric_limits<double>::lowest();
    double maxValue = std::numeric_limits<double>::max();
};
