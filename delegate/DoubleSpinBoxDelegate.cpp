#include "DoubleSpinBoxDelegate.h"

#include <QDoubleSpinBox>

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double min, double max, int decimals, double singleStep, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_singleStep(singleStep)
{}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setDecimals(m_decimals);
    editor->setSingleStep(m_singleStep);
    editor->setFrame(false);

    return editor;
}

QString DoubleSpinBoxDelegate::displayText(const QVariant &value, const QLocale &) const
{
    return QString::number(value.toDouble(), 'f', m_decimals);
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    double value = index.data(Qt::EditRole).toDouble();

    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}