#pragma once

#include <QStyledItemDelegate>

class ColorIntervalDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ColorIntervalDelegate(QObject *parent = nullptr);
    
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem &option, const QModelIndex &index) override;
};
