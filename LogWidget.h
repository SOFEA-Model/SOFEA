#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include "Utilities.h"

#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>

#include <QAction>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

class LogWidgetBackend;

// Custom Filter Proxy
class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LogFilterProxyModel(QObject *parent = nullptr);
    void setFilterTag(const QString& tag, bool visible);
    void setInfoVisible(bool visible);
    void setWarningsVisible(bool visible);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QMap<QString, bool> tags;
    bool infoVisible = true;
    bool warningsVisible = true;
};

// Main Widget
class LogWidget : public QWidget
{
    Q_OBJECT

friend class LogWidgetBackend;

public:
    explicit LogWidget(QWidget *parent = nullptr);
    void clear();

private:
    void init();
    void appendRow(const QString& text, boost::log::trivial::severity_level severity, const QString &tag);

    QAction *clearAct;
    QAction *showInfoAct;
    QAction *showWarningsAct;
    QAction *filterDistributionAct;
    QAction *filterGeometryAct;
    QAction *filterModelAct;
    QAction *filterAnalysisAct;

    QToolBar *toolbar;
    QStandardItemModel *logModel;
    LogFilterProxyModel *proxyModel;
    QTreeView *logView;
};

// Custom Backend for Boost.Log v2
class LogWidgetBackend
    : public boost::log::sinks::basic_formatted_sink_backend<char, boost::log::sinks::synchronized_feeding>
{
public:
    explicit LogWidgetBackend(LogWidget *log);
    void consume(const boost::log::record_view& rec, const string_type& fstring);

private:
    LogWidget *m_widget;
};

#endif // LOGWIDGET_H
