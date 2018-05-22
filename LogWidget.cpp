#include "LogWidget.h"

#include <QHeaderView>
#include <QStylePainter>

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

void LogFilterProxyModel::setFilterTag(const QString& tag, bool visible)
{
    tags[tag] = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setInfoVisible(bool visible)
{
    infoVisible = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setWarningsVisible(bool visible)
{
    warningsVisible = visible;
    invalidateFilter();
}

bool LogFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    QModelIndex index = sourceModel()->index(row, 0, parent);

    int severity = sourceModel()->data(index, Qt::UserRole).toInt();
    QString tag = sourceModel()->data(index, Qt::UserRole + 1).toString();

    bool accept = true;
    if (severity <= boost::log::trivial::info && !infoVisible)
        accept = false;
    if (severity == boost::log::trivial::warning && !warningsVisible)
        accept = false;

    // Exclude any tags where visibility is explicitly set to false.
    accept &= tags.value(tag, true);

    return accept;
}

//-----------------------------------------------------------------------------
// LogWidget
//-----------------------------------------------------------------------------

LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    const QIcon clearIcon = QIcon(":/images/CleanData_16x.png");
    const QIcon infoIcon = QIcon(":/images/StatusAnnotations_Information_16xLG.png");
    const QIcon warningIcon = QIcon(":/images/StatusAnnotations_Warning_16xLG.png");
    const QIcon filterIcon = QIcon(":/images/FilterDropdown_16x.png");

    clearAct = new QAction(clearIcon, tr("Clear"), this);
    clearAct->setStatusTip("Clear messages");

    showInfoAct = new QAction(infoIcon, tr("Show Information"), this);
    showInfoAct->setCheckable(true);

    showWarningsAct = new QAction(warningIcon, tr("Show Warnings"), this);
    showWarningsAct->setCheckable(true);

    filterDistributionAct = new QAction(tr("Distribution"), this);
    filterGeometryAct = new QAction(tr("Geometry"), this);
    filterModelAct = new QAction(tr("Model"), this);
    filterAnalysisAct = new QAction(tr("Analysis"), this);

    filterDistributionAct->setCheckable(true);
    filterGeometryAct->setCheckable(true);
    filterModelAct->setCheckable(true);
    filterAnalysisAct->setCheckable(true);

    QMenu *filterMenu = new QMenu;
    filterMenu->addAction(filterDistributionAct);
    filterMenu->addAction(filterGeometryAct);
    filterMenu->addAction(filterModelAct);
    filterMenu->addAction(filterAnalysisAct);

    FilterButton *filterBtn = new FilterButton;
    filterBtn->setToolTip(tr("Filter"));
    filterBtn->setIcon(filterIcon);
    filterBtn->setMenu(filterMenu);
    filterBtn->setPopupMode(QToolButton::InstantPopup);

    toolbar = new QToolBar;
    toolbar->setIconSize(QSize(16,16));
    toolbar->addAction(clearAct);
    toolbar->addSeparator();
    toolbar->addAction(showInfoAct);
    toolbar->addAction(showWarningsAct);
    toolbar->addSeparator();
    toolbar->addWidget(filterBtn);

    // Model
    logModel = new QStandardItemModel;
    proxyModel = new LogFilterProxyModel(this);
    proxyModel->setSourceModel(logModel);

    // View
    logView = new QTreeView;
    logView->setModel(proxyModel);
    logView->header()->hide();
    logView->setRootIsDecorated(false);
    logView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    logView->setSelectionBehavior(QAbstractItemView::SelectRows);
    logView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolbar, 0);
    mainLayout->addWidget(logView, 1);

    setLayout(mainLayout);
    init();
}

void LogWidget::clear()
{
    logModel->clear();
}

void LogWidget::init()
{
    // Setup Connections
    connect(clearAct, &QAction::triggered, this, &LogWidget::clear);
    connect(showInfoAct, &QAction::triggered, proxyModel, &LogFilterProxyModel::setInfoVisible);
    connect(showWarningsAct, &QAction::triggered, proxyModel, &LogFilterProxyModel::setWarningsVisible);
    connect(filterDistributionAct, &QAction::toggled, [=](bool checked) {
        proxyModel->setFilterTag("Distribution", checked);
    });
    connect(filterGeometryAct, &QAction::toggled, [=](bool checked) {
        proxyModel->setFilterTag("Geometry", checked);
    });
    connect(filterModelAct, &QAction::toggled, [=](bool checked) {
        proxyModel->setFilterTag("Model", checked);
    });
    connect(filterAnalysisAct, &QAction::toggled, [=](bool checked) {
        proxyModel->setFilterTag("Analysis", checked);
    });

    // Set Defaults
    showInfoAct->setChecked(true);
    showWarningsAct->setChecked(true);
    filterDistributionAct->setChecked(true);
    filterGeometryAct->setChecked(true);
    filterModelAct->setChecked(true);
    filterAnalysisAct->setChecked(true);
}

void LogWidget::appendRow(const QString& text, boost::log::trivial::severity_level severity, const QString& tag)
{
    static const QMap<boost::log::trivial::severity_level, QIcon> iconMap = {
        {boost::log::trivial::trace,   QIcon(":/images/StatusAnnotations_Information_16xLG.png")},
        {boost::log::trivial::debug,   QIcon(":/images/StatusAnnotations_Information_16xLG.png")},
        {boost::log::trivial::info,    QIcon(":/images/StatusAnnotations_Information_16xLG_color.png")},
        {boost::log::trivial::warning, QIcon(":/images/StatusAnnotations_Warning_16xLG_color.png")},
        {boost::log::trivial::error,   QIcon(":/images/StatusAnnotations_Invalid_16xLG_color.png")},
        {boost::log::trivial::fatal,   QIcon(":/images/StatusAnnotations_Critical_16xLG_color.png")}
    };

    QStandardItem *item = new QStandardItem;
    item->setData(text, Qt::DisplayRole);
    item->setData(severity, Qt::UserRole);
    item->setData(tag, Qt::UserRole + 1);
    item->setData(iconMap[severity], Qt::DecorationRole);

    logModel->appendRow(item);
}

//-----------------------------------------------------------------------------
// LogWidgetBackend
//-----------------------------------------------------------------------------

// Custom Backend for Boost.Log v2
LogWidgetBackend::LogWidgetBackend(LogWidget *widget) : m_widget(widget)
{
}

void LogWidgetBackend::consume(const boost::log::record_view& rec, const string_type& fstring)
{
    typedef boost::log::trivial::severity_level severity_level;

    QString text = QString::fromStdString(fstring);
    boost::log::value_ref<severity_level> severity = boost::log::extract<severity_level>("Severity", rec);

    QString tag;
    if (rec.attribute_values().count("Tag") > 0) {
        boost::log::value_ref<std::string> source = boost::log::extract<std::string>("Tag", rec);
        tag = QString::fromStdString(source.get());
    }

    m_widget->appendRow(text, severity.get(), tag);
}
