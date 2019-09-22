#include "LogWidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QStylePainter>
#include <QVBoxLayout>

#include <QDebug>

#include <boost/log/utility/value_ref.hpp>

//-----------------------------------------------------------------------------
// FilterButton
//-----------------------------------------------------------------------------

// Workaround for QTBUG-2036
class FilterButton : public QToolButton
{
public:
    FilterButton(QWidget *parent = nullptr) : QToolButton(parent)
    {
    }

protected:
    virtual void paintEvent(QPaintEvent *) override
    {
        QStylePainter p(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.features &= (~QStyleOptionToolButton::HasMenu);
        p.drawComplexControl(QStyle::CC_ToolButton, opt);
    }
};

//-----------------------------------------------------------------------------
// LogFilterProxyModel
//-----------------------------------------------------------------------------

LogFilterProxyModel::LogFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void LogFilterProxyModel::setTagVisible(const QString& tag, bool visible)
{
    tagVisibility[tag] = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setErrorsVisible(bool visible)
{
    errorsVisible = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setMessagesVisible(bool visible)
{
    messagesVisible = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setWarningsVisible(bool visible)
{
    warningsVisible = visible;
    invalidateFilter();
}

bool LogFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    // Severity is the UserRole associated with column 0.
    QModelIndex severityIndex = sourceModel()->index(row, 0, parent);
    int severity = sourceModel()->data(severityIndex, Qt::UserRole).toInt();

    QModelIndex tagIndex = sourceModel()->index(row, filterKeyColumn(), parent);
    QString tag = sourceModel()->data(tagIndex, Qt::DisplayRole).toString();

    bool accept = true;
    switch (severity) {
    case boost::log::trivial::trace:
    case boost::log::trivial::debug:
    case boost::log::trivial::info:
        accept = messagesVisible;
        break;
    case boost::log::trivial::warning:
        accept = warningsVisible;
        break;
    case boost::log::trivial::error:
    case boost::log::trivial::fatal:
        accept = errorsVisible;
        break;
    default:
        break;
    }

    // Exclude any tags where visibility is explicitly set to false.
    accept &= tagVisibility.value(tag, true);

    return accept;
}

//-----------------------------------------------------------------------------
// LogWidget
//-----------------------------------------------------------------------------

LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    // Toolbar Items
    static const QIcon clearIcon = QIcon(":/images/CleanData_24x.png");
    static const QIcon errorsIcon = QIcon(":/images/StatusAnnotations_Critical_24xLG_color.png");
    static const QIcon warningsIcon = QIcon(":/images/StatusAnnotations_Warning_24xLG_color.png");
    static const QIcon messagesIcon = QIcon(":/images/StatusAnnotations_Information_24xLG_color.png");
    static const QIcon filterIcon = QIcon(":/images/FilterDropdown_24x.png");

    clearAct = new QAction(clearIcon, tr("Clear"), this);
    showErrorsAct = new QAction(errorsIcon, tr("0 Errors"), this);
    showErrorsAct->setCheckable(true);
    showWarningsAct = new QAction(warningsIcon, tr("0 Warnings"), this);
    showWarningsAct->setCheckable(true);
    showMessagesAct = new QAction(messagesIcon, tr("0 Messages"), this);
    showMessagesAct->setCheckable(true);
    filterMenu = new QMenu;

    FilterButton *filterButton = new FilterButton;
    filterButton->setToolTip(tr("Filter"));
    filterButton->setIcon(filterIcon);
    filterButton->setMenu(filterMenu);
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
        {boost::log::trivial::trace,   QIcon(":/images/StatusAnnotations_Information_16xLG.png")},
        {boost::log::trivial::debug,   QIcon(":/images/StatusAnnotations_Information_16xLG.png")},
        {boost::log::trivial::info,    QIcon(":/images/StatusAnnotations_Information_16xLG_color.png")},
        {boost::log::trivial::warning, QIcon(":/images/StatusAnnotations_Warning_16xLG_color.png")},
        {boost::log::trivial::error,   QIcon(":/images/StatusAnnotations_Invalid_16xLG_color.png")},
        {boost::log::trivial::fatal,   QIcon(":/images/StatusAnnotations_Critical_16xLG_color.png")}
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
