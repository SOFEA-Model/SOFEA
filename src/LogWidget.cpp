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

#include "AppStyle.h"
#include "LogWidget.h"
#include "models/LogFilterProxyModel.h"

#include <QAction>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>
#include <QStylePainter>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <QDebug>

#include <boost/log/utility/value_ref.hpp>

LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    // Toolbar Items

    const QIcon clearIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionCleanData));
    const QIcon errorsIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusCritical));
    const QIcon warningsIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusWarning));
    const QIcon messagesIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInformation));
    const QIcon filterIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_FilterDropdownButton));

    clearAct = new QAction(clearIcon, tr("Clear"), this);
    showErrorsAct = new QAction(errorsIcon, tr("0 Errors"), this);
    showErrorsAct->setCheckable(true);
    showWarningsAct = new QAction(warningsIcon, tr("0 Warnings"), this);
    showWarningsAct->setCheckable(true);
    showMessagesAct = new QAction(messagesIcon, tr("0 Messages"), this);
    showMessagesAct->setCheckable(true);
    filterMenu = new QMenu;

    filterButton = new QToolButton;
    filterButton->setToolTip(tr("Filter"));
    filterButton->setIcon(filterIcon);
    filterButton->setMenu(filterMenu);
    filterButton->setArrowType(Qt::NoArrow);
    filterButton->setPopupMode(QToolButton::InstantPopup);

    // Toolbar
    toolbar = new QToolBar;
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    toolbar->setIconSize(QSize(iconSize, iconSize));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(showErrorsAct);
    toolbar->addAction(showWarningsAct);
    toolbar->addAction(showMessagesAct);
    toolbar->addSeparator();
    toolbar->addAction(clearAct);
    toolbar->addSeparator();
    toolbar->addWidget(filterButton);

    // Model
    logModel = new QStandardItemModel;
    proxyModel = new LogFilterProxyModel(this);
    proxyModel->setSourceModel(logModel);

    // View
    logView = new QTreeView;
    logView->setModel(proxyModel);
    logView->setRootIsDecorated(false);
    logView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    logView->setSelectionBehavior(QAbstractItemView::SelectRows);
    logView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView *header = logView->header();
    header->setFont(QApplication::font());
    header->setFixedHeight(32);
    header->setStretchLastSection(false);
    header->setSectionResizeMode(QHeaderView::Interactive);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolbar, 0);
    mainLayout->addWidget(logView, 1);

    setLayout(mainLayout);
    init();
}

void LogWidget::setFilterKeyColumn(int column)
{
    filterKeyColumn = column;
    proxyModel->setFilterKeyColumn(column);
}

void LogWidget::setFilterValues(const QStringList& values)
{
    // This should only be called once.
    if (!filterMenu->isEmpty())
        return;

    for (const QString& value : values) {
        QAction *filterAct = new QAction(value, this);
        filterAct->setCheckable(true);
        filterAct->setChecked(true);
        filterMenu->addAction(filterAct);
    }

    // Set the connections. Need to iterate over the menu after
    // it has taken ownership of the pointer.
    for (const QAction *act : filterMenu->actions()) {
        connect(act, &QAction::toggled, [=](bool checked) {
            proxyModel->setTagVisible(act->text(), checked);
        });
    }
}

void LogWidget::setUniformRowHeights(bool uniform)
{
    logView->setUniformRowHeights(uniform);
}

void LogWidget::clear()
{
    logModel->removeRows(0, logModel->rowCount());

    messageCount = 0;
    warningCount = 0;
    errorCount = 0;

    showMessagesAct->setText("0 Messages");
    showWarningsAct->setText("0 Warnings");
    showErrorsAct->setText("0 Errors");
}

void LogWidget::init()
{
    // Setup Connections
    connect(clearAct, &QAction::triggered, this, &LogWidget::clear);
    connect(showErrorsAct, &QAction::triggered, proxyModel, &LogFilterProxyModel::setErrorsVisible);
    connect(showWarningsAct, &QAction::triggered, proxyModel, &LogFilterProxyModel::setWarningsVisible);
    connect(showMessagesAct, &QAction::triggered, proxyModel, &LogFilterProxyModel::setMessagesVisible);

    // Set Defaults
    showErrorsAct->setChecked(true);
    showWarningsAct->setChecked(true);
    showMessagesAct->setChecked(true);
}

void LogWidget::setColumn(int column, const QString& label, const QString& attribute, int size, bool hide)
{
    QStandardItem *headerItem = new QStandardItem;
    headerItem->setData(label, Qt::DisplayRole);
    headerItem->setData(attribute, Qt::UserRole);
    logModel->setHorizontalHeaderItem(column, headerItem);

    QHeaderView* headerView = logView->header();
    if (size > 0) {
        headerView->setSectionResizeMode(column, QHeaderView::Fixed);
        headerView->resizeSection(column, size);
    }
    else {
        headerView->setSectionResizeMode(column, QHeaderView::Stretch);
    }

    logView->setColumnHidden(column, hide);
}

void LogWidget::appendRow(const QString& text, const QHash<QString, QVariant>& attrs, boost::log::trivial::severity_level severity)
{
    static const QHash<boost::log::trivial::severity_level, QIcon> icons = {
        {boost::log::trivial::trace,   this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusDebug))},
        {boost::log::trivial::debug,   this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusDebug))},
        {boost::log::trivial::info,    this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInformation))},
        {boost::log::trivial::warning, this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusWarning))},
        {boost::log::trivial::error,   this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusInvalid))},
        {boost::log::trivial::fatal,   this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusCritical))}
    };

    QList<QStandardItem *> items;

    QStandardItem *messageItem = new QStandardItem;
    messageItem->setData(text, Qt::DisplayRole);
    messageItem->setData(icons[severity], Qt::DecorationRole);
    messageItem->setData(severity, Qt::UserRole);
    items.append(messageItem);

    for (int i = 1; i < logModel->columnCount(); ++i) {
        QStandardItem *headerItem = logModel->horizontalHeaderItem(i);
        QString attrKey = headerItem->data(Qt::UserRole).toString();
        QVariant attr = attrs.value(attrKey);
        QStandardItem *item = new QStandardItem;
        item->setData(attr, Qt::DisplayRole);
        items.append(item);
    }

    // Update filter buttons
    switch (severity) {
    case boost::log::trivial::trace:
    case boost::log::trivial::debug:
    case boost::log::trivial::info:
        showMessagesAct->setText(QString("%1 Messages").arg(++messageCount));
        break;
    case boost::log::trivial::warning:
        showWarningsAct->setText(QString("%1 Warnings").arg(++warningCount));
        break;
    case boost::log::trivial::error:
    case boost::log::trivial::fatal:
        showErrorsAct->setText(QString("%1 Errors").arg(++errorCount));
        break;
    default:
        break;
    }

    logModel->appendRow(items);
}

//-----------------------------------------------------------------------------
// LogWidgetBackend
//-----------------------------------------------------------------------------

// Custom Backend for Boost.Log v2
LogWidgetBackend::LogWidgetBackend(LogWidget *widget) : m_widget(widget)
{
}

void LogWidgetBackend::setKeywords(const QStringList& keywords)
{
    // Keywords for attribute extraction.
    m_keywords = keywords;
}

void LogWidgetBackend::consume(const boost::log::record_view& rec, const string_type& fstring)
{
    typedef boost::log::trivial::severity_level severity_level;

    // Expected stored value types are string and integer.
    typedef boost::mpl::vector<std::string, int> types;

    // Extract the record text.
    QString text = QString::fromStdString(fstring);
    text = text.simplified();

    // Extract severity attribute.
    boost::log::value_ref<severity_level> severity = boost::log::extract<severity_level>("Severity", rec);
    if (!severity)
        return;

    // Extract additional attributes by keyword.
    QHash<QString, QVariant> attrs;

    for (const QString& keyword : m_keywords)
    {
        const char* cstr = keyword.toLocal8Bit().constData();
        boost::log::value_ref<types> val = boost::log::extract<types>(cstr, rec);
        if (val) {
            switch (val.which()) {
            case 0: {
                QVariant v(QString::fromStdString(val.get<std::string>()));
                attrs[keyword] = v;
                break;
            }
            case 1: {
                QVariant v(val.get<int>());
                attrs[keyword] = v;
                break;
            }
            }
        }
    }

    m_widget->appendRow(text, attrs, severity.get());
}
