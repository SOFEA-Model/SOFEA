#pragma once

#include <QDateTime>
#include <QStyledItemDelegate>

class DateTimeEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DateTimeEditDelegate(QDateTime min, QDateTime max, QObject *parent = nullptr);
    
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const QDateTime m_min;
    const QDateTime m_max;
};
