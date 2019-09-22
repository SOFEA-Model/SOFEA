#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>

#include <QAction>
#include <QComboBox>
#include <QHash>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

class LogWidgetBackend;

//-----------------------------------------------------------------------------
// LogFilterProxyModel
//-----------------------------------------------------------------------------

// Custom Filter Proxy
class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LogFilterProxyModel(QObject *parent = nullptr);
    void setTagVisible(const QString& tag, bool visible);
    void setErrorsVisible(bool visible);
    void setWarningsVisible(bool visible);
    void setMessagesVisible(bool visible);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QHash<QString, bool> tagVisibility;
    bool errorsVisible = true;
    bool warningsVisible = true;
    bool messagesVisible = true;
};

//-----------------------------------------------------------------------------
// LogWidget
//-----------------------------------------------------------------------------

class LogWidget : public QWidget
{
    Q_OBJECT

friend class LogWidgetBackend;

public:
    explicit LogWidget(QWidget *parent = nullptr);
    void setColumn(int column, const QString& label, const QString& attribute, int size = 0, bool hide = false);
    void setFilterKeyColumn(int column);
    void setFilterValues(const QStringList& values);
    void setUniformRowHeights(bool uniform);
    void clear();

private:
    void init();
    void appendRow(const QString& text, const QHash<QString, QVariant> &attrs, boost::log::trivial::severity_level severity);

    QAction *clearAct;
    QAction *showErrorsAct;
    QAction *showWarningsAct;
    QAction *showMessagesAct;
    QMenu *filterMenu;
    QToolBar *toolbar;
    QStandardItemModel *logModel;
    LogFilterProxyModel *proxyModel;
    QTreeView *logView;

    int filterKeyColumn = 0;
    int errorCount = 0;
    int warningCount = 0;
    int messageCount = 0;
};

//-----------------------------------------------------------------------------
// LogWidgetBackend
//-----------------------------------------------------------------------------

// Custom Backend for Boost.Log v2
class LogWidgetBackend
    : public boost::log::sinks::basic_formatted_sink_backend<char, boost::log::sinks::synchronized_feeding>
{
public:
    explicit LogWidgetBackend(LogWidget *log);
    void setKeywords(const QStringList& keywords);
    void consume(const boost::log::record_view& rec, const string_type& fstring);

private:
    LogWidget *m_widget;
    QStringList m_keywords;
};

#endif // LOGWIDGET_H
