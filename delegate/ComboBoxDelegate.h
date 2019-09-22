#pragma once

#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(QAbstractItemModel *model, int column, QObject *parent = nullptr);
    
    void setEditorModel(QAbstractItemModel *model);
    void setEditorModelColumn(int column);
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index);

signals:
    void editRequested();

private slots:
    void emitCommitData();

private:
    QAbstractItemModel *m_editorModel;
    int m_editorColumn;
};
