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
#include "DateTimeDistributionDialog.h"
#include "core/DateTimeConversion.h"
#include "delegate/DoubleSpinBoxDelegate.h"

#include <utility>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_map.hpp>

#include <QBoxLayout>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>

#include <QDebug>

DateTimeDistributionDialog::DateTimeDistributionDialog(const DateTimeDistribution &d, QWidget *parent)
    : QDialog(parent), currentDist(d)
{
    setWindowTitle(tr("Define Distribution"));
    setWindowFlag(Qt::Tool);
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_LineEditFunctionIcon)));

    dteLower = new QDateTimeEdit;
    dteLower->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteLower->setTimeSpec(Qt::UTC);
    dteLower->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    dteUpper = new QDateTimeEdit;
    dteUpper->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteUpper->setTimeSpec(Qt::UTC);
    dteUpper->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    model = new QStandardItemModel;
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels(QStringList{"Interval", "Probability"});

    view = new StandardTableView;
    view->setModel(model);
    view->setMinimumWidth(450);
    view->setItemDelegateForColumn(1, new DoubleSpinBoxDelegate(0.0001, 1, 4, 0.1));
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);

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
    lblStatus->setStatusType(StatusLabel::Alert);
    lblStatus->setText("Probabilities have been normalized.");
    lblStatus->hide();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &DateTimeDistributionDialog::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DateTimeDistributionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DateTimeDistributionDialog::reject);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->addRow(new QLabel("Lower:"), dteLower);
    inputLayout->addRow(new QLabel("Upper:"), dteUpper);

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
        break;
    }
    case 1: {
        DiscreteDateTime *d = boost::get<DiscreteDateTime>(&currentDist);
        if (d) {
            intervals = *d;
            resetTable();

            // Initialize the DateTimeEdits with the first interval lower bound.
            if (!intervals.empty()) {
                auto i = *intervals.begin();
                QDateTime dt = sofea::utilities::convert<QDateTime>(i.first.lower());
                dteLower->setDateTime(dt);
                dteUpper->setDateTime(dt);
            }
        }
        break;
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
    double probability = 1.0;

    // Update the interval map.
    ptime ptlower = sofea::utilities::convert<ptime>(dtlower);
    ptime ptupper = sofea::utilities::convert<ptime>(dtupper);
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
        std::string is = sofea::utilities::convert<std::string>(i.first);

        int currentRow = model->rowCount();
        model->insertRow(currentRow);
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(QString::fromStdString(is), Qt::DisplayRole);
        item1->setData(i.second, Qt::DisplayRole);
        model->setItem(currentRow, 0, item0);
        model->setItem(currentRow, 1, item1);
        item0->setFlags(item0->flags() & ~Qt::ItemIsEnabled);
    }

    lblStatus->hide();
}

void DateTimeDistributionDialog::apply()
{
    // If interval map is empty, set constant distribution from lower bound.
    if (intervals.empty()) {
        Distribution::ConstantDateTime d;
        d.setTimeSpec(Qt::UTC);
        d.setDate(dteLower->date());
        d.setTime(dteLower->time());
        currentDist = d;
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
}

void DateTimeDistributionDialog::accept()
{
    apply();
    QDialog::done(QDialog::Accepted);
}

void DateTimeDistributionDialog::reject()
{
    QDialog::done(QDialog::Rejected);
}
