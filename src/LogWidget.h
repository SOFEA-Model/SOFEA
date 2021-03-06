// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>

#include <QHash>
#include <QString>
#include <QStringList>
#include <QWidget>

class LogFilterProxyModel;
class LogWidgetBackend;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QStandardItemModel;
class QToolBar;
class QToolButton;
class QTreeView;
QT_END_NAMESPACE

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
    QToolButton *filterButton;
    QToolBar *toolbar;
    QStandardItemModel *logModel;
    LogFilterProxyModel *proxyModel;
    QTreeView *logView;

    int filterKeyColumn = 0;
    int errorCount = 0;
    int warningCount = 0;
    int messageCount = 0;
};

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
