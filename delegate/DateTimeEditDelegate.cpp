#include "DateTimeEditDelegate.h"

#include <QDateTimeEdit>

DateTimeEditDelegate::DateTimeEditDelegate(QDateTime min, QDateTime max, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max)
{}

QWidget *DateTimeEditDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setDateTimeRange(m_min, m_max);
    editor->setTimeSpec(Qt::UTC);
    editor->setDisplayFormat("yyyy-MM-dd HH:mm");
    editor->setFrame(false);

    return editor;
}

void DateTimeEditDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    QDateTime value = index.data(Qt::EditRole).toDateTime();
    value.setTimeSpec(Qt::UTC);
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit *>(editor);
    dtEdit->setDateTime(value);
}

void DateTimeEditDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit *>(editor);
    dtEdit->interpretText();
    QDateTime value = dtEdit->dateTime();
    value.setTimeSpec(Qt::UTC);
    model->setData(index, value, Qt::EditRole);
}

void DateTimeEditDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
