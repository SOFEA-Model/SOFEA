#pragma once

#include <QStyledItemDelegate>

class PointDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PointDelegate(QObject *parent = nullptr);
    
    QString displayText(const QVariant &value, const QLocale &) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};
