#pragma once

#include <QStyledItemDelegate>

class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SpinBoxDelegate(QObject *parent = nullptr);
    
    SpinBoxDelegate(int min, int max, int singleStep, QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;
    QString displayText(const QVariant &, const QLocale &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const int m_min;
    const int m_max;
    const int m_singleStep;
};
