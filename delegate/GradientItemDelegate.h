#pragma once

#include <QLinearGradient>
#include <QMetaType>
#include <QPixmap>
#include <QStyledItemDelegate>

class GradientItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GradientItemDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    static QPixmap gradientPixmap(const QLinearGradient& gradient);
};

Q_DECLARE_METATYPE(QLinearGradient)
