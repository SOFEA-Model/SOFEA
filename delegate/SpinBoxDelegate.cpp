#include "SpinBoxDelegate.h"

#include <QSpinBox>

SpinBoxDelegate::SpinBoxDelegate(int min, int max, int singleStep, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_singleStep(singleStep)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem&,
    const QModelIndex&) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setSingleStep(m_singleStep);
    editor->setFrame(false);

    return editor;
}

QString SpinBoxDelegate::displayText(const QVariant& value, const QLocale&) const
{
    return QString::number(value.toInt());
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    int value = index.data(Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex& index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem& option,
    const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}