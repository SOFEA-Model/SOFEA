#include "DoubleItemDelegate.h"

#include <QDoubleValidator>
#include <QLineEdit>

#include <limits>

constexpr double low_double = std::numeric_limits<double>::lowest();
constexpr double max_double = std::numeric_limits<double>::max();

DoubleItemDelegate::DoubleItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_min(low_double), m_max(max_double), m_decimals(15), m_fixed(false)
{}

DoubleItemDelegate::DoubleItemDelegate(double min, double max, int decimals, bool fixed, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_fixed(fixed)
{}

QWidget *DoubleItemDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
	QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    QDoubleValidator *validator = new QDoubleValidator(m_min, m_max, m_decimals);
	editor->setValidator(validator);
	return editor;
}

void DoubleItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    double value = index.data(Qt::EditRole).toDouble();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    if (m_fixed)
        lineEdit->setText(QString::number(value, 'f', m_decimals));
    else
        lineEdit->setText(QString::number(value, 'g', m_decimals));
}

QString DoubleItemDelegate::displayText(const QVariant &value, const QLocale &) const
{
    if (m_fixed)
        return QString::number(value.toDouble(), 'f', m_decimals);
    else
        return QString::number(value.toDouble(), 'g', m_decimals);
}