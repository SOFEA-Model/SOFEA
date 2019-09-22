#ifndef RECEPTORDELEGATE_H
#define RECEPTORDELEGATE_H

#include <QApplication>
#include <QAbstractItemModel>
#include <QColorDialog>
#include <QFont>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QRegExpValidator>
#include <QString>
#include <QStyledItemDelegate>

#include "delegate/ColorPickerDelegate.h"

//-----------------------------------------------------------------------------
// ReceptorGridDelegate
//-----------------------------------------------------------------------------

class ReceptorGridDelegate : public QStyledItemDelegate
{
public:
    explicit ReceptorGridDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        // Draw a vertical gridline on the left, except in the first column.
        // Horizontal gridlines are drawn in ReceptorTreeView::drawRow.
        if (index.column() == 0)
            return;

        QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
        painter->save();
        painter->setPen(QPen(color));
        int x = (option.direction == Qt::LeftToRight) ? option.rect.left() : option.rect.right();
        painter->drawLine(x, option.rect.y(), x, option.rect.bottom());
        painter->restore();
    }
};

//-----------------------------------------------------------------------------
// ReceptorGroupDelegate
//-----------------------------------------------------------------------------

class ReceptorGroupDelegate : public ReceptorGridDelegate
{
public:
    explicit ReceptorGroupDelegate(QObject *parent = nullptr)
        : ReceptorGridDelegate(parent)
    {}

protected:
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        // Limit to alphanumeric characters, maximum length 8
        QLineEdit *editor = new QLineEdit(parent);
        editor->setFrame(false);
        editor->setMaxLength(8);
        QFont font = editor->font();
        font.setCapitalization(QFont::AllUppercase);
        editor->setFont(font);
        QRegExp rx("[A-Za-z0-9]+");
        QRegExpValidator *validator = new QRegExpValidator(rx, editor);
        editor->setValidator(validator);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex& index) const override
    {
        QString value = index.data(Qt::EditRole).toString().toUpper();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        editor->setGeometry(option.rect);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;

        // Disable focus rect for children.
        if (index.internalId() != 0)
            opt.state = opt.state & (~QStyle::State_HasFocus);

        QStyledItemDelegate::paint(painter, opt, index);
    }
};

//-----------------------------------------------------------------------------
// ReceptorColorDelegate
//-----------------------------------------------------------------------------

class ReceptorColorDelegate : public ReceptorGridDelegate
{
public:
    explicit ReceptorColorDelegate(QObject *parent = nullptr)
        : ReceptorGridDelegate(parent)
    {}

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index) override
    {
        if (event->type() != QEvent::MouseButtonDblClick)
            return false;

        // Editing only enabled for parent.
        if (index.internalId() != 0)
            return false;

        QColor color = index.data(Qt::DisplayRole).value<QColor>();
        QColorDialog dialog(color);
        dialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
        int rc = dialog.exec();
        if (rc != QDialog::Accepted)
            return false;

        color = dialog.currentColor();
        model->setData(index, color, Qt::EditRole);
        return true;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        // Disable focus rect for children.
        if (index.internalId() != 0) {
            QStyleOptionViewItem opt = option;
            opt.state = option.state & (~QStyle::State_HasFocus);
            QStyledItemDelegate::paint(painter, opt, index);
            return;
        }

        // TODO: QwtSymbol::drawSymbol
        QColor color = index.data(Qt::DisplayRole).value<QColor>();
        QPixmap pixmap = ColorPickerDelegate::brushValuePixmap(QBrush(color));

        const int x = option.rect.center().x() - pixmap.rect().width() / 2;
        const int y = option.rect.center().y() - pixmap.rect().height() / 2;

        // Draw background when selected.
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        else
            painter->fillRect(option.rect, option.palette.window());

        painter->drawPixmap(QRect(x, y, pixmap.rect().width(), pixmap.rect().height()), pixmap);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
        return QSize(iconSize, iconSize);
    }
};


#endif // RECEPTORDELEGATE_H
