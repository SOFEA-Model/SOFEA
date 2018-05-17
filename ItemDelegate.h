#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QDateTime>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>

//-----------------------------------------------------------------------------
// ColorPickerDelegate
//-----------------------------------------------------------------------------

class ColorPickerDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ColorPickerDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

private:
    static QPixmap brushValuePixmap(const QBrush &b);
};

//-----------------------------------------------------------------------------
// DoubleItemDelegate
//-----------------------------------------------------------------------------

class DoubleItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DoubleItemDelegate(QObject *parent = nullptr);
    DoubleItemDelegate(double min, double max, int decimals, bool fixed = false,
                       QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &) const override;    

    QString displayText(const QVariant &, const QLocale &) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private:
    const double m_min;
    const double m_max;
    const int m_decimals;
    const bool m_fixed;
};

//-----------------------------------------------------------------------------
// SpinBoxDelegate
//-----------------------------------------------------------------------------

class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent = nullptr);
    SpinBoxDelegate(int min, int max, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;

    QString displayText(const QVariant &, const QLocale &) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const int m_min;
    const int m_max;
};

//-----------------------------------------------------------------------------
// DoubleSpinBoxDelegate
//-----------------------------------------------------------------------------

class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DoubleSpinBoxDelegate(QObject *parent = nullptr);
    DoubleSpinBoxDelegate(double min, double max, int decimals, bool fixed = false,
                          QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const override;

    QString displayText(const QVariant &, const QLocale &) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const double m_min;
    const double m_max;
    const int m_decimals;
    const bool m_fixed;
};

//-----------------------------------------------------------------------------
// DateTimeEditDelegate
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// PointDelegate
//-----------------------------------------------------------------------------

class PointDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PointDelegate(QObject *parent = nullptr);

    QString displayText(const QVariant &value, const QLocale &) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};


//-----------------------------------------------------------------------------
// ProgressBarDelegate
//-----------------------------------------------------------------------------

class ProgressBarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ProgressBarDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

#endif // ITEMDELEGATE_H
