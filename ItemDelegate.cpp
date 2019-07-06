#include "ItemDelegate.h"
#include "PixmapUtilities.h"

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
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
    QColor color = index.data(Qt::DisplayRole).value<QColor>();
    QColorDialog *dialog = new QColorDialog(parent);
    dialog->setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dialog->setCurrentColor(color);
    return dialog;
}

void ColorPickerDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    QColorDialog *dialog = qobject_cast<QColorDialog *>(editor);
    QColor color = index.data(Qt::EditRole).value<QColor>();
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
    QColor color = index.data(Qt::DisplayRole).value<QColor>();
    QPixmap pixmap = PixmapUtilities::brushValuePixmap(QBrush(color));

    const int x = option.rect.center().x() - pixmap.rect().width() / 2;
    const int y = option.rect.center().y() - pixmap.rect().height() / 2;

    // Draw background when selected.
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    painter->drawPixmap(QRect(x, y, pixmap.rect().width(), pixmap.rect().height()), pixmap);
}

QSize ColorPickerDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(16, 16);
}

//-----------------------------------------------------------------------------
// GradientItemDelegate
//-----------------------------------------------------------------------------

GradientItemDelegate::GradientItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void GradientItemDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::DecorationRole).canConvert<QLinearGradient>()) {
        QLinearGradient gradient = qvariant_cast<QLinearGradient>(index.data(Qt::DecorationRole));
        QPixmap pixmap = PixmapUtilities::gradientPixmap(gradient);

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        const int x = option.rect.x();
        const int y = option.rect.y();

        painter->drawPixmap(QRect(x, y, pixmap.rect().width(), pixmap.rect().height()), pixmap);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize GradientItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(48, 16);
}

//-----------------------------------------------------------------------------
// ColorIntervalDelegate
//-----------------------------------------------------------------------------

// TODO: ColorIntervalModel
// - Store current min, max range in UserRole for editor
// - Model calculates displayText

ColorIntervalDelegate::ColorIntervalDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ColorIntervalDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    //editor->setRange(m_min, m_max);
    //editor->setDecimals(m_decimals);
    //editor->setSingleStep(m_singleStep);
    editor->setFrame(false);

    return editor;
}

void ColorIntervalDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    double value = index.data(Qt::EditRole).toDouble();
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void ColorIntervalDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void ColorIntervalDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}


bool ColorIntervalDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem &option, const QModelIndex &index)
{
    bool hasDecoration = option.features & QStyleOptionViewItem::HasDecoration;
    if (!hasDecoration)
        return false;

    if (event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint pos = me->pos() - option.rect.topLeft();

        const QStyle *s = QApplication::style();
        QRect iconRect = s->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
        if (iconRect.contains(pos)) {
            qDebug() << "Decoration Hit";
            // Open QColorDialog
            return true;
        }
        else {
            qDebug() << "Other Hit";
            return false;
        }
    }

    return false;
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

//-----------------------------------------------------------------------------
// SpinBoxDelegate
//-----------------------------------------------------------------------------

SpinBoxDelegate::SpinBoxDelegate(int min, int max, int singleStep, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_singleStep(singleStep)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setSingleStep(m_singleStep);
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
    int value = index.data(Qt::EditRole).toInt();

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

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double min, double max, int decimals, double singleStep, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_singleStep(singleStep)
{
}

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
    QDateTime value = index.data(Qt::EditRole).toDateTime();
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

ComboBoxDelegate::ComboBoxDelegate(QAbstractItemModel *model, int column, QObject *parent)
    : QStyledItemDelegate(parent), m_editorModel(model), m_editorColumn(column)
{
}

void ComboBoxDelegate::setEditorModel(QAbstractItemModel *model)
{
    m_editorModel = model;
}

void ComboBoxDelegate::setEditorModelColumn(int column)
{
    m_editorColumn = column;
}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    QComboBox *cb = new QComboBox(parent);
    cb->setFrame(false);
    cb->setModel(m_editorModel);
    cb->setModelColumn(m_editorColumn);

    connect(cb, QOverload<int>::of(&QComboBox::activated),
            this, &ComboBoxDelegate::emitCommitData);

    return cb;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
       int cbIndex = cb->findData(index.data(Qt::EditRole), Qt::EditRole);
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
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, cb->currentData(Qt::EditRole), Qt::EditRole);
        // The current index is stored as Qt::UserRole.
        model->setData(index, cb->currentIndex(), Qt::UserRole);
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

bool ComboBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    emit editRequested();
    return false;
}

void ComboBoxDelegate::emitCommitData()
{
    // Notify other application components of the change.
    emit commitData(qobject_cast<QWidget *>(sender()));
}

//-----------------------------------------------------------------------------
// PointDelegate
//-----------------------------------------------------------------------------

PointDelegate::PointDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
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

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

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

        // Use base background fill even when selected.
        if (option.state & QStyle::State_Selected) {
            if (option.features & QStyleOptionViewItem::Alternate)
                painter->fillRect(option.rect, option.palette.alternateBase());
            else
                painter->fillRect(option.rect, option.palette.base());
        }

        // Draw a CE_ProgressBar without CE_ProgressBarGroove.
        QApplication::style()->drawControl(QStyle::CE_ProgressBarContents, &progressBarOption, painter);
        QApplication::style()->drawControl(QStyle::CE_ProgressBarLabel, &progressBarOption, painter);
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
