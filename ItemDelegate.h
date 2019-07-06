#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QDateTime>
#include <QLinearGradient>
#include <QPushButton>
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
};

//-----------------------------------------------------------------------------
// GradientItemDelegate
//-----------------------------------------------------------------------------

class GradientItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GradientItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

Q_DECLARE_METATYPE(QLinearGradient)

//-----------------------------------------------------------------------------
// ColorIntervalDelegate
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// DoubleItemDelegate
//-----------------------------------------------------------------------------

class DoubleItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DoubleItemDelegate(QObject *parent = nullptr);
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
    explicit SpinBoxDelegate(QObject *parent = nullptr);
    SpinBoxDelegate(int min, int max, int singleStep, QObject *parent = nullptr);
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
    const int m_singleStep;
};

//-----------------------------------------------------------------------------
// DoubleSpinBoxDelegate
//-----------------------------------------------------------------------------

class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DoubleSpinBoxDelegate(double min, double max, int decimals, double singleStep,
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
    const double m_singleStep;
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
// ComboBoxDelegate
//-----------------------------------------------------------------------------

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
    explicit ProgressBarDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

#endif // ITEMDELEGATE_H
