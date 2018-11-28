#include <sstream>
#include <locale>
#include <utility>

#include <QBoxLayout>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItemModel>
#include <QDebug>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_map.hpp>

#include "DateTimeDistributionDialog.h"

inline boost::posix_time::ptime asPosixTime(const QDateTime& dt)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    int y = dt.date().year();
    int m = dt.date().month();
    int d = dt.date().day();
    int h = dt.time().hour();

    ptime pt(date(y, m, d), hours(h));
    return pt;
}

inline QDateTime asDateTime(const boost::posix_time::ptime& pt)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    date gd = pt.date();
    time_duration td = pt.time_of_day();

    return QDateTime(QDate(gd.year(), gd.month(), gd.day()),
                     QTime(td.hours(), 0, 0), Qt::UTC);
}

inline QString intervalString(const boost::icl::discrete_interval<boost::posix_time::ptime>& i)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    std::ostringstream oss;
    ptime lower = i.lower();
    ptime upper = i.upper();
    time_period period(lower, upper);

    time_facet *facet = new time_facet(); // std::locale handles destruction
    facet->format("%Y-%m-%d %H:%M");
    period_formatter formatter(period_formatter::AS_OPEN_RANGE, ", ", "[", ")", "]");
    facet->period_formatter(formatter);
    oss.imbue(std::locale(std::locale::classic(), facet));

    oss << period;
    return QString::fromStdString(oss.str());
}

//-----------------------------------------------------------------------------
// DateTimeDistributionDialog
//-----------------------------------------------------------------------------

DateTimeDistributionDialog::DateTimeDistributionDialog(const DateTimeDistribution &d, QWidget *parent)
    : QDialog(parent), currentDist(d)
{
    setWindowTitle(tr("Define Distribution"));

    dteLower = new QDateTimeEdit;
    dteLower->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteLower->setTimeSpec(Qt::UTC);
    dteLower->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    dteUpper = new QDateTimeEdit;
    dteUpper->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteUpper->setTimeSpec(Qt::UTC);
    dteUpper->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    sbProbability = new QDoubleSpinBox;
    sbProbability->setRange(0.0001, 1);
    sbProbability->setValue(1);
    sbProbability->setDecimals(4);
    sbProbability->setSingleStep(0.01);

    model = new QStandardItemModel;
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels(QStringList{"Interval", "Probability"});

    view = new StandardTableView;
    view->setModel(model);
    view->setMinimumWidth(450);
    view->setDoubleSpinBoxForColumn(1, 0.0001, 1, 4, true);
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView *header = view->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::Fixed);
    header->resizeSection(1, 100);

    btnAdd = new QPushButton("Add");
    btnClear = new QPushButton("Clear");
    connect(btnAdd,   &QPushButton::clicked, this, &DateTimeDistributionDialog::onAddClicked);
    connect(btnClear, &QPushButton::clicked, this, &DateTimeDistributionDialog::onClearClicked);

    lblStatus = new StatusLabel;
    lblStatus->setSeverity(2);
    lblStatus->setText("Probabilities have been normalized.");
    lblStatus->hide();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DateTimeDistributionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DateTimeDistributionDialog::reject);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->addRow(new QLabel("Lower:"), dteLower);
    inputLayout->addRow(new QLabel("Upper:"), dteUpper);
    inputLayout->addRow(new QLabel("Probability:"), sbProbability);

    QVBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->setMargin(0);
    controlsLayout->addWidget(btnAdd);
    controlsLayout->addWidget(btnClear);
    controlsLayout->addStretch(1);

    QHBoxLayout *tableLayout = new QHBoxLayout;
    tableLayout->setMargin(0);
    tableLayout->addWidget(view);
    tableLayout->addLayout(controlsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(tableLayout);
    mainLayout->addWidget(lblStatus);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    init();
}

void DateTimeDistributionDialog::init()
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;
    using namespace Distribution;

    switch (currentDist.which()) {
    case 0: {
        ConstantDateTime *d = boost::get<ConstantDateTime>(&currentDist);
        if (d) {
            QDateTime dt(d->date(), d->time(), Qt::UTC);
            dteLower->setDateTime(dt);
            dteUpper->setDateTime(dt);
        }
    }
    case 1: {
        DiscreteDateTime *d = boost::get<DiscreteDateTime>(&currentDist);
        if (d) {
            intervals = *d;
            resetTable();

            // Initialize the DateTimeEdits with the first interval lower bound.
            if (!intervals.empty()) {
                auto i = *intervals.begin();
                QDateTime dt = asDateTime(i.first.lower());
                dteLower->setDateTime(dt);
                dteUpper->setDateTime(dt);
            }
        }
    }
    }
}

DateTimeDistribution DateTimeDistributionDialog::getDistribution() const
{
    return currentDist;
}

void DateTimeDistributionDialog::onAddClicked()
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;

    QDateTime dtlower = dteLower->dateTime();
    QDateTime dtupper = dteUpper->dateTime();
    double probability = sbProbability->value();

    // Update the interval map.
    ptime ptlower = asPosixTime(dtlower);
    ptime ptupper = asPosixTime(dtupper);
    auto di = discrete_interval<ptime>::right_open(ptlower, ptupper);
    intervals += std::make_pair(di, probability);

    resetTable();
}

void DateTimeDistributionDialog::onClearClicked()
{
    // Reset the interval map.
    intervals.clear();
    resetTable();
}

void DateTimeDistributionDialog::resetTable()
{
    model->removeRows(0, model->rowCount());
    for (const auto& i : intervals)
    {
        int currentRow = model->rowCount();
        model->insertRow(currentRow);
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(intervalString(i.first), Qt::DisplayRole);
        item1->setData(i.second, Qt::DisplayRole);
        model->setItem(currentRow, 0, item0);
        model->setItem(currentRow, 1, item1);
    }

    lblStatus->hide();
}

void DateTimeDistributionDialog::accept()
{
    // If interval map is empty, set constant distribution from lower bound.
    if (intervals.empty()) {
        Distribution::ConstantDateTime d;
        d.setTimeSpec(Qt::UTC);
        d.setDate(dteLower->date());
        d.setTime(dteLower->time());
        currentDist = d;
        saved = true;
        return;
    }

    // Normalize probabilities.
    const double sum = std::accumulate(intervals.begin(), intervals.end(), 0.0,
        [](auto &a, auto &i) { return a + i.second; });
    for (auto &i : intervals)
        i.second = i.second / sum;

    // Update table with new probabilities.
    resetTable();
    if (sum != 1) lblStatus->show();

    currentDist = Distribution::DiscreteDateTime(intervals);
    saved = true;
}

void DateTimeDistributionDialog::reject()
{
    if (saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}
