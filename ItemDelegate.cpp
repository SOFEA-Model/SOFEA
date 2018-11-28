#include "ItemDelegate.h"

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QSpinBox>

#include <QDebug>

#include <limits>

constexpr double low_double = std::numeric_limits<double>::lowest();
constexpr double max_double = std::numeric_limits<double>::max();

//-----------------------------------------------------------------------------
// ColorPickerDelegate
//-----------------------------------------------------------------------------

ColorPickerDelegate::ColorPickerDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ColorPickerDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &index) const
{
    QColor color = index.model()->data(index, Qt::DisplayRole).value<QColor>();
    QColorDialog *dialog = new QColorDialog(parent);
    dialog->setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dialog->setCurrentColor(color);
    return dialog;
}

void ColorPickerDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    QColorDialog *dialog = qobject_cast<QColorDialog *>(editor);
    QColor color = index.model()->data(index, Qt::EditRole).value<QColor>();
    dialog->setCurrentColor(color);
}

void ColorPickerDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model, const QModelIndex &index) const
{
    QColorDialog *dialog = qobject_cast<QColorDialog *>(editor);
    if (dialog->result() == QDialog::Accepted) {
        QColor color = dialog->currentColor();
        model->setData(index, color, Qt::EditRole);
    }
}

void ColorPickerDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color = index.model()->data(index, Qt::DisplayRole).value<QColor>();
    QPixmap pixmap = brushValuePixmap(QBrush(color));

    const int x = option.rect.center().x() - pixmap.rect().width() / 2;
    const int y = option.rect.center().y() - pixmap.rect().height() / 2;

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(QRect(x, y, pixmap.rect().width(), pixmap.rect().height()), pixmap);
}

QSize ColorPickerDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(16, 16);
}

QPixmap ColorPickerDelegate::brushValuePixmap(const QBrush &b)
{
    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), b);
    QColor color = b.color();
    if (color.alpha() != 255) { // indicate alpha by an inset
        QBrush  opaqueBrush = b;
        color.setAlpha(255);
        opaqueBrush.setColor(color);
        painter.fillRect(img.width() / 4, img.height() / 4,
                         img.width() / 2, img.height() / 2, opaqueBrush);
    }
    painter.end();
    return QPixmap::fromImage(img);
}

//-----------------------------------------------------------------------------
// DoubleItemDelegate
//-----------------------------------------------------------------------------

DoubleItemDelegate::DoubleItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_min(low_double), m_max(max_double), m_decimals(15), m_fixed(false)
{
}

DoubleItemDelegate::DoubleItemDelegate(double min, double max, int decimals, bool fixed, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_fixed(fixed)
{
}

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
    double value = index.model()->data(index, Qt::EditRole).toDouble();
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

//-----------------------------------------------------------------------------
// SpinBoxDelegate
//-----------------------------------------------------------------------------

SpinBoxDelegate::SpinBoxDelegate(int min, int max, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setFrame(false);

    return editor;
}

QString SpinBoxDelegate::displayText(const QVariant &value, const QLocale &) const
{
    return QString::number(value.toInt());
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

//-----------------------------------------------------------------------------
// DoubleSpinBoxDelegate
//-----------------------------------------------------------------------------

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_min(0.0), m_max(100.00), m_decimals(2), m_fixed(false)
{
}

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double min, double max, int decimals, bool fixed, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_fixed(fixed)
{
}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setDecimals(m_decimals);
    editor->setFrame(false);

    return editor;
}

QString DoubleSpinBoxDelegate::displayText(const QVariant &value, const QLocale &) const
{
    if (m_fixed)
        return QString::number(value.toDouble(), 'f', m_decimals);
    else
        return QString::number(value.toDouble(), 'g', m_decimals);
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();

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

//-----------------------------------------------------------------------------
// DateTimeEditDelegate
//-----------------------------------------------------------------------------

DateTimeEditDelegate::DateTimeEditDelegate(QDateTime min, QDateTime max, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max)
{
}

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
    QDateTime value = index.model()->data(index, Qt::EditRole).toDateTime();
    value.setTimeSpec(Qt::UTC);
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit*>(editor);
    dtEdit->setDateTime(value);
}

void DateTimeEditDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit*>(editor);
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

//-----------------------------------------------------------------------------
// ComboBoxDelegate
//-----------------------------------------------------------------------------

ComboBoxDelegate::ComboBoxDelegate(QAbstractItemModel *sourceModel, QObject *parent)
    : QStyledItemDelegate(parent), m_sourceModel(sourceModel)
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    QComboBox *cb = new QComboBox(parent);
    cb->setModel(m_sourceModel);
    return cb;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
       QString currentText = index.data(Qt::EditRole).toString();
       int cbIndex = cb->findText(currentText);
       if (cbIndex >= 0)
           cb->setCurrentIndex(cbIndex);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex& index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor))
        // save the current text of the combo box as the current value of the item
        model->setData(index, cb->currentText(), Qt::EditRole);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}

//-----------------------------------------------------------------------------
// PointDelegate
//-----------------------------------------------------------------------------

PointDelegate::PointDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QString PointDelegate::displayText(const QVariant &value, const QLocale &) const
{
    if (value.type() == QMetaType::QPoint) {
        QPoint p = value.toPoint();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else if (value.type() == QMetaType::QPointF) {
        QPointF p = value.toPointF();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else {
        return QString();
    }
}

void PointDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    // Workaround for QPoint(0,0) which returns isNull() == true
    if ((index.data(Qt::DisplayRole).type() == QVariant::Point) ||
        (index.data(Qt::DisplayRole).type() == QVariant::PointF))
    {
        painter->drawText(option.rect,
                          displayText(index.data(), QLocale::system()),
                          option.displayAlignment);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

//-----------------------------------------------------------------------------
// ProgressBarDelegate
//-----------------------------------------------------------------------------

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    if (index.data().canConvert<int>())
    {
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        int progress = index.data(Qt::DisplayRole).toInt();
        progressBarOption.progress = progress < 0 ? 0 : progress;
        progressBarOption.text = QString::asprintf("%d%%", progressBarOption.progress);

        // Draw the progress bar.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
