#pragma once

#include <QStyledItemDelegate>

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
