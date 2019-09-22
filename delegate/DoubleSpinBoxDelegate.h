#pragma once

#include <QStyledItemDelegate>

class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DoubleSpinBoxDelegate(double min, double max, int decimals, double singleStep,
                          QObject *parent = nullptr);
                          
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;
    QString displayText(const QVariant &, const QLocale &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const double m_min;
    const double m_max;
    const int m_decimals;
    const double m_singleStep;
};
