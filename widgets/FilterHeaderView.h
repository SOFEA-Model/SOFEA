#ifndef FILTERHEADERVIEW_H
#define FILTERHEADERVIEW_H

#include <QHeaderView>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTreeView>
#include <QAction>
#include <QWidgetAction>

#include <map>

class FilterHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    FilterHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setAutoFilterEnabled(bool enabled);
    bool autoFilterEnabled() const;

private slots:
    void showAutoFilter(int section);

protected:
    virtual void leaveEvent(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect, int section) const override;

private:
    enum class SortFilterState
    {
        None,
        SortAscending,
        SortAscendingFilter,
        SortDescending,
        SortDescendingFilter,
        Filter
    };

    bool isPointOnIcon(int section, const QPoint& pos) const;
    void paintIndicator(QPainter *painter, const QRect &rect, SortFilterState state) const;

    QAction *sortAscendingAct;
    QAction *sortDescendingAct;
    QAction *clearFilterAct;
    QAction *editFilterAct;
    //QTreeView *filterTree;
    //QWidgetAction *filterAction;

    bool autoFilter = false;
    int hoverSection = -1;
    std::map<int, SortFilterState> columnStateMap;
};

#endif // FILTERHEADERVIEW_H
