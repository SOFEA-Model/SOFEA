#pragma once

#include <QAbstractItemDelegate>
#include <QBrush>
#include <QPixmap>

class ColorPickerDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit ColorPickerDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index) override;

    static QPixmap brushValuePixmap(const QBrush& brush);
};
