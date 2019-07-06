#include <QtWidgets>

#include <array>
#include <limits>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/optional.hpp>

#define BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include "GenericDistributionDialog.h"
#include "Utilities.h"

enum class DistributionType
{
    Discrete,
    Continuous,
    Step
};

enum class DistributionCategory
{
    Uniform,
    Bernoulli,
    Poisson,
    Normal,
    Sampling,
    Miscellaneous
};

using ID = DistributionID;
using DT = DistributionType;
using DC = DistributionCategory;

// FIXME: replace with multi-index containers

const QMap<DistributionCategory, QString> categoryMap {
    { DC::Uniform,       "Uniform Distributions"       },
    { DC::Bernoulli,     "Bernoulli Distributions"     },
    { DC::Poisson,       "Poisson Distributions"       },
    { DC::Normal,        "Normal Distributions"        },
    { DC::Sampling,      "Sampling Distributions"      },
    { DC::Miscellaneous, "Miscellaneous Distributions" }
};

// Reverse Sort
const QMultiMap<DistributionCategory, DistributionID> categoryToIDs {
    { DC::Uniform,        ID::UniformReal       },
    { DC::Uniform,        ID::UniformInt        },
    { DC::Bernoulli,      ID::NegativeBinomial  },
    { DC::Bernoulli,      ID::Geometric         },
    { DC::Bernoulli,      ID::Binomial          },
    { DC::Poisson,        ID::Laplace           },
    { DC::Poisson,        ID::Beta              },
    { DC::Poisson,        ID::ExtremeValue      },
    { DC::Poisson,        ID::Weibull           },
    { DC::Poisson,        ID::Gamma             },
    { DC::Poisson,        ID::Exponential       },
    { DC::Poisson,        ID::Poisson           },
    { DC::Normal,         ID::StudentT          },
    { DC::Normal,         ID::FisherF           },
    { DC::Normal,         ID::Cauchy            },
    { DC::Normal,         ID::NCChiSquared      },
    { DC::Normal,         ID::ChiSquared        },
    { DC::Normal,         ID::Lognormal         },
    { DC::Normal,         ID::Normal            },
    { DC::Sampling,       ID::PiecewiseLinear   },
    { DC::Sampling,       ID::PiecewiseConstant },
    { DC::Sampling,       ID::Discrete          },
    { DC::Miscellaneous,  ID::Triangle          }
};

struct DistributionInfo
{
    DistributionCategory category;
    DistributionType type;
    QString fullName;
};

const QMap<DistributionID, DistributionInfo> distributionMap {
    { ID::UniformReal,        { DC::Uniform,        DT::Continuous,  "Uniform Distribution (Real)"         }},
    { ID::UniformInt,         { DC::Uniform,        DT::Discrete,    "Uniform Distribution (Int)"          }},
    { ID::NegativeBinomial,   { DC::Bernoulli,      DT::Discrete,    "Negative Binomial Distribution"      }},
    { ID::Geometric,          { DC::Bernoulli,      DT::Discrete,    "Geometric Distribution"              }},
    { ID::Binomial,           { DC::Bernoulli,      DT::Discrete,    "Binomial Distribution"               }},
    { ID::Laplace,            { DC::Poisson,        DT::Continuous,  "Laplace Distribution"                }},
    { ID::Beta,               { DC::Poisson,        DT::Continuous,  "Beta Distribution"                   }},
    { ID::ExtremeValue,       { DC::Poisson,        DT::Continuous,  "Extreme Value Distribution"          }},
    { ID::Weibull,            { DC::Poisson,        DT::Continuous,  "Weibull Distribution"                }},
    { ID::Gamma,              { DC::Poisson,        DT::Continuous,  "Gamma (Erlang) Distribution"         }},
    { ID::Exponential,        { DC::Poisson,        DT::Continuous,  "Exponential Distribution"            }},
    { ID::Poisson,            { DC::Poisson,        DT::Discrete,    "Poisson Distribution"                }},
    { ID::StudentT,           { DC::Normal,         DT::Continuous,  "Student's t Distribution"            }},
    { ID::FisherF,            { DC::Normal,         DT::Continuous,  "F Distribution"                      }},
    { ID::Cauchy,             { DC::Normal,         DT::Continuous,  "Cauchy-Lorentz Distribution"         }},
    { ID::NCChiSquared,       { DC::Normal,         DT::Continuous,  "Noncentral Chi-Squared Distribution" }},
    { ID::ChiSquared,         { DC::Normal,         DT::Continuous,  "Chi Squared Distribution"            }},
    { ID::Lognormal,          { DC::Normal,         DT::Continuous,  "Log Normal Distribution"             }},
    { ID::Normal,             { DC::Normal,         DT::Continuous,  "Normal (Gaussian) Distribution"      }},
    { ID::PiecewiseLinear,    { DC::Sampling,       DT::Continuous,  "Piecewise Linear Distribution"       }},
    { ID::PiecewiseConstant,  { DC::Sampling,       DT::Step,        "Piecewise Constant Distribution"     }},
    { ID::Discrete,           { DC::Sampling,       DT::Discrete,    "Discrete Distribution"               }},
    { ID::Triangle,           { DC::Miscellaneous,  DT::Continuous,  "Triangular Distribution"             }}
};

struct ParameterInfo
{
    QString name;
    boost::optional<double> value;
    boost::optional<double> min;
    boost::optional<double> max;
    int precision;
    double step;
};

// Reverse Sort
const QMultiMap<ID, ParameterInfo> parameterMap = {
//    Distribution             Name                               Value       Minimum       Maximum      Decimals    Step
//    ---------------------    ------------------------     -----------   -----------   -----------   -----------    ----
    { ID::UniformInt,        { "Upper: ",                   boost::none,  boost::none,  boost::none,            0,      1 }},
    { ID::UniformInt,        { "Lower: ",                   boost::none,  boost::none,  boost::none,            0,      1 }},
    { ID::UniformReal,       { "Upper: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::UniformReal,       { "Lower: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::Binomial,          { "Probability of success: ",  boost::none,     0.000001,        1.000,            6,   0.01 }},
    { ID::Binomial,          { "Trials: ",                  boost::none,            1,  boost::none,            0,      1 }},
    { ID::Geometric,         { "Probability: ",             boost::none,     0.000001,        0.999,            6,   0.01 }},
    { ID::NegativeBinomial,  { "Probability of success: ",  boost::none,     0.000001,        0.999,            6,   0.01 }},
    { ID::NegativeBinomial,  { "Successes: ",               boost::none,            1,  boost::none,            0,      1 }},
    { ID::Beta,              { "Beta: ",                    boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Beta,              { "Alpha: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Exponential,       { "Rate: ",                    boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::ExtremeValue,      { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::ExtremeValue,      { "Location: ",                boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Gamma,             { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Gamma,             { "Shape: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Laplace,           { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Laplace,           { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::Poisson,           { "Mean: ",                    boost::none,     0.000001,  boost::none,            6,      1 }},
    { ID::Weibull,           { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Weibull,           { "Shape: ",                   boost::none,            1,  boost::none,            6,   0.01 }},
    { ID::Cauchy,            { "Scale: ",                   boost::none,            0,  boost::none,            6,   0.01 }},
    { ID::Cauchy,            { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::ChiSquared,        { "Degrees of freedom: ",      boost::none,            2,         1000,            0,      1 }},
    { ID::NCChiSquared,      { "Non-centrality: ",          boost::none,     0.000001,         1000,            6,   0.01 }},
    { ID::NCChiSquared,      { "Degrees of freedom: ",      boost::none,            1,         1000,            0,      1 }},
    { ID::FisherF,           { "Degrees of freedom 2: ",    boost::none,            1,          100,            0,      1 }},
    { ID::FisherF,           { "Degrees of freedom 1: ",    boost::none,            2,           20,            0,      1 }},
    { ID::Lognormal,         { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Lognormal,         { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::Normal,            { "Standard deviation: ",      boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { ID::Normal,            { "Mean: ",                    boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::StudentT,          { "Degrees of freedom: ",      boost::none,            1,          100,            0,      1 }},
    { ID::Triangle,          { "Upper: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::Triangle,          { "Mode: ",                    boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { ID::Triangle,          { "Lower: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }}
};

// Remaps parameters of the Boost.Random distribution to ParameterInfo.
// TODO: store vector of functions? e.g.
// std::vector<boost::function<void>> functions;
// functions.push_back(&a));
// functions.push_back(&b));

struct ParamVisitor : public boost::static_visitor<>
{
    void operator()(const Distribution::UniformReal d) {
        id = ID::UniformReal;
        paramList = parameterMap.values(ID::UniformReal);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::UniformInt d) {
        id = ID::UniformInt;
        paramList = parameterMap.values(ID::UniformInt);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::Binomial d) {
        id = ID::Binomial;
        paramList = parameterMap.values(ID::Binomial);
        paramList[0].value = d.t();
        paramList[1].value = d.p();
    }
    void operator()(const Distribution::Geometric d) {
        id = ID::Geometric;
        paramList = parameterMap.values(ID::Geometric);
        paramList[0].value = d.p();
    }
    void operator()(const Distribution::NegativeBinomial d) {
        id = ID::NegativeBinomial;
        paramList = parameterMap.values(ID::NegativeBinomial);
        paramList[0].value = d.k();
        paramList[1].value = d.p();
    }
    void operator()(const Distribution::Poisson d) {
        id = ID::Poisson;
        paramList = parameterMap.values(ID::Poisson);
        paramList[0].value = d.mean();
    }
    void operator()(const Distribution::Exponential d) {
        id = ID::Exponential;
        paramList = parameterMap.values(ID::Exponential);
        paramList[0].value = d.lambda();
    }
    void operator()(const Distribution::Gamma d) {
        id = ID::Gamma;
        paramList = parameterMap.values(ID::Gamma);
        paramList[0].value = d.alpha();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Weibull d) {
        id = ID::Weibull;
        paramList = parameterMap.values(ID::Weibull);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::ExtremeValue d) {
        id = ID::ExtremeValue;
        paramList = parameterMap.values(ID::ExtremeValue);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::Beta d) {
        id = ID::Beta;
        paramList = parameterMap.values(ID::Beta);
        paramList[0].value = d.alpha();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Laplace d) {
        id = ID::Laplace;
        paramList = parameterMap.values(ID::Laplace);
        paramList[0].value = d.mean();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Normal d) {
        id = ID::Normal;
        paramList = parameterMap.values(ID::Normal);
        paramList[0].value = d.mean();
        paramList[1].value = d.sigma();
    }
    void operator()(const Distribution::Lognormal d) {
        id = ID::Lognormal;
        paramList = parameterMap.values(ID::Lognormal);
        paramList[0].value = d.m();
        paramList[1].value = d.s();
    }
    void operator()(const Distribution::ChiSquared d) {
        id = ID::ChiSquared;
        paramList = parameterMap.values(ID::ChiSquared);
        paramList[0].value = d.n();
    }
    void operator()(const Distribution::NCChiSquared d) {
        id = ID::NCChiSquared;
        paramList = parameterMap.values(ID::NCChiSquared);
        paramList[0].value = d.k();
        paramList[1].value = d.lambda();
    }
    void operator()(const Distribution::Cauchy d) {
        id = ID::Cauchy;
        paramList = parameterMap.values(ID::Cauchy);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::FisherF d) {
        id = ID::FisherF;
        paramList = parameterMap.values(ID::FisherF);
        paramList[0].value = d.m();
        paramList[1].value = d.n();
    }
    void operator()(const Distribution::StudentT d) {
        id = ID::StudentT;
        paramList = parameterMap.values(ID::StudentT);
        paramList[0].value = d.n();
    }
    void operator()(const Distribution::Discrete d) {
        id = ID::Discrete;
    }
    void operator()(const Distribution::PiecewiseConstant d) {
        id = ID::PiecewiseConstant;
    }
    void operator()(const Distribution::PiecewiseLinear d) {
        id = ID::PiecewiseLinear;
    }
    void operator()(const Distribution::Triangle d) {
        id = ID::Triangle;
        paramList = parameterMap.values(ID::Triangle);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
        paramList[2].value = d.c();
    }
    void operator()(const Distribution::Constant d) {}

    DistributionID id;
    QList<ParameterInfo> paramList;
};

// Sets reasonable default limits for the PDF plot, based on current parameters.

struct PdfLimitsVisitor : public boost::static_visitor<>
{
    void operator()(const Distribution::UniformReal d) {
        x0 = d.a() - (d.b() - d.a());
        x1 = d.b() + (d.b() - d.a());
    }
    void operator()(const Distribution::UniformInt d) {
        x0 = d.a() - (d.b() - d.a());
        x1 = d.b() + (d.b() - d.a());
    }
    void operator()(const Distribution::Binomial d) {
        x0 = 0;
        x1 = d.t();
    }
    void operator()(const Distribution::Geometric d) {
        x0 = 1;
        x1 = 10;
    }
    void operator()(const Distribution::NegativeBinomial d) {
        x0 = 0;
        x1 = 25;
    }
    void operator()(const Distribution::Poisson d) {
        x0 = 0;
        x1 = 2 * d.mean();
    }
    void operator()(const Distribution::Exponential d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::Gamma d) {
        x0 = 0;
        x1 = d.alpha() * 2;
    }
    void operator()(const Distribution::Weibull d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::ExtremeValue d) {
        x0 = d.a() - (4 * d.b());
        x1 = d.a() + (6 * d.b());
    }
    void operator()(const Distribution::Beta d) {
        x0 = 0.01;
        x1 = 0.99;
    }
    void operator()(const Distribution::Laplace d) {
        x0 = d.mean()- (4 * d.beta());
        x1 = d.mean() + (4 * d.beta());
    }
    void operator()(const Distribution::Normal d) {
        x0 = d.mean() - 5;
        x1 = d.mean() + 5;
    }
    void operator()(const Distribution::Lognormal d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::ChiSquared d) {
        x0 = 0;
        x1 = 8;
    }
    void operator()(const Distribution::NCChiSquared d) {
        x0 = 0;
        x1 = 8;
    }
    void operator()(const Distribution::Cauchy d) {
        x0 = d.a() - 5;
        x1 = d.a() + 5;
    }
    void operator()(const Distribution::FisherF d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::StudentT d) {
        x0 = -5;
        x1 = 5;
    }
    void operator()(const Distribution::Discrete d) {}
    void operator()(const Distribution::PiecewiseConstant d) {}
    void operator()(const Distribution::PiecewiseLinear d) {}
    void operator()(const Distribution::Triangle d) {
        x1 = d.c();
        x0 = d.a();
    }
    void operator()(const Distribution::Constant d) {}

    double x0;
    double x1;
};

GenericDistributionDialog::GenericDistributionDialog(const GenericDistribution& d, QWidget *parent)
    : QDialog(parent), currentDist(d)
{
    setWindowTitle(tr("Define Distribution"));
    setWindowFlag(Qt::Tool);
    setWindowIcon(QIcon(":/images/Effects_32x.png"));

    distributionTree = new QTreeView;
    distributionTree->setHeaderHidden(true);
    distributionTree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    label1 = new QLabel;
    label2 = new QLabel;
    label3 = new QLabel;

    sb1 = new QDoubleSpinBox;
    sb2 = new QDoubleSpinBox;
    sb3 = new QDoubleSpinBox;

    sb1->setMinimum(std::numeric_limits<double>::lowest());
    sb2->setMinimum(std::numeric_limits<double>::lowest());
    sb3->setMinimum(std::numeric_limits<double>::lowest());
    sb1->setMaximum(std::numeric_limits<double>::max());
    sb2->setMaximum(std::numeric_limits<double>::max());
    sb3->setMaximum(std::numeric_limits<double>::max());

    pdfPlot = new StandardPlot;
    pdfPlot->setXAxisTitle("Value");
    pdfPlot->setYAxisTitle("Density");
    //pdfPlot->setAxisScale(QwtPlot::yLeft, 0, 1);

    pdfPlotCurveData = new QwtPointSeriesData;
    pdfPlotCurve = new QwtPlotCurve;
    pdfPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    pdfPlotCurve->attach(pdfPlot);

    QFormLayout *controlsLayout = new QFormLayout;
    controlsLayout->addRow(label1, sb1);
    controlsLayout->addRow(label2, sb2);
    controlsLayout->addRow(label3, sb3);
    QFrame *controlsFrame = new QFrame;
    controlsFrame->setLayout(controlsLayout);
    controlsFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QPalette controlsFramePalette = controlsFrame->palette();
    controlsFramePalette.setColor(QPalette::Window, Qt::white);
    controlsFrame->setPalette(controlsFramePalette);
    controlsFrame->setAutoFillBackground(true);

    // Button Box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Distribution Controls
    QSplitter *controlsSplitter = new QSplitter;
    controlsSplitter->setOrientation(Qt::Vertical);
    controlsSplitter->setMinimumWidth(300);
    controlsSplitter->addWidget(distributionTree);
    controlsSplitter->addWidget(controlsFrame);

    // Plot Frame
    QFrame *plotFrame = new QFrame;
    QVBoxLayout *plotFrameLayout = new QVBoxLayout;
    plotFrameLayout->addWidget(pdfPlot);
    plotFrame->setLayout(plotFrameLayout);
    plotFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QPalette plotFramePalette = plotFrame->palette();
    plotFramePalette.setColor(QPalette::Window, Qt::white);
    plotFrame->setPalette(plotFramePalette);
    plotFrame->setAutoFillBackground(true);

    // Splitter
    QSplitter *mainSplitter = new QSplitter;
    mainSplitter->addWidget(controlsSplitter);
    mainSplitter->addWidget(plotFrame);
    mainSplitter->setOrientation(Qt::Horizontal);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    initializeModel();
    initializeConnections();
    resetControls();
    updatePlot();
}

void GenericDistributionDialog::initializeModel()
{
    distributionModel = new QStandardItemModel;
    auto keys = categoryToIDs.uniqueKeys();
    for (const auto &i : keys) {
        QString categoryStr = categoryMap.value(i);
        QStandardItem *rootItem = new QStandardItem(categoryStr);
        rootItem->setData(-1, Qt::UserRole);
        for (const auto &id : categoryToIDs.values(i)) {
            DistributionInfo info = distributionMap.value(id);
            QStandardItem *item = new QStandardItem(info.fullName);
            item->setData(static_cast<int>(id), Qt::UserRole); // DistributionID
            rootItem->appendRow(item);
        }
        distributionModel->appendRow(rootItem);

        // FontRole
        rootItem->setSelectable(false);
        QFont font = qvariant_cast<QFont>(rootItem->data(Qt::FontRole));
        font.setBold(true);
        rootItem->setData(font, Qt::FontRole);
    }

    distributionTree->setModel(distributionModel);
}

void GenericDistributionDialog::initializeConnections()
{
    connect(distributionTree->selectionModel(), &QItemSelectionModel::currentChanged,
        [=](const QModelIndex& currentIndex, const QModelIndex&)
    {
        QStandardItem *currentItem = distributionModel->itemFromIndex(currentIndex);
        int id = qvariant_cast<int>(currentItem->data(Qt::UserRole)); // DistributionID

        if (id >= 0) {
            currentID = static_cast<DistributionID>(id);
            setDistribution(currentID);
            resetControls();
            updatePlot();
        }
    });

    connect(sb1, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GenericDistributionDialog::onValueChanged);
    connect(sb2, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GenericDistributionDialog::onValueChanged);
    connect(sb3, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &GenericDistributionDialog::onValueChanged);
}

void GenericDistributionDialog::resetControls()
{
    ParamVisitor pv;
    boost::apply_visitor(pv, currentDist);

    currentID = pv.id;

    // Iterate over the distribution model, select current
    for (int i = 0; i < distributionModel->rowCount(); ++i) {
        QModelIndex parentIndex = distributionModel->index(i, 0);
        if (distributionModel->hasChildren(parentIndex)) {
            for (int j = 0; j < distributionModel->rowCount(parentIndex); ++j) {
                QModelIndex index = distributionModel->index(j, 0, parentIndex);
                QStandardItem *item = distributionModel->itemFromIndex(index);
                int id = qvariant_cast<int>(item->data(Qt::UserRole));
                if (id == static_cast<int>(pv.id)) {
                    distributionTree->expand(parentIndex);
                    distributionTree->selectionModel()->select(index, QItemSelectionModel::Select);
                }
            }
        }
    }

    int n = pv.paramList.length();

    double min = std::numeric_limits<double>::lowest();
    double max = std::numeric_limits<double>::max();

    label1->setVisible(false);
    label2->setVisible(false);
    label3->setVisible(false);

    sb1->setVisible(false);
    sb2->setVisible(false);
    sb3->setVisible(false);

    if (n > 0) {
        ParameterInfo pi1 = pv.paramList[0];
        label1->setText(pi1.name);
        sb1->setMinimum(pi1.min.value_or(min));
        sb1->setMaximum(pi1.max.value_or(max));
        sb1->setDecimals(pi1.precision);
        sb1->setSingleStep(pi1.step);
        sb1->setValue(pi1.value.value());
        label1->setVisible(true);
        sb1->setVisible(true);
    }
    if (n > 1) {
        ParameterInfo pi2 = pv.paramList[1];
        label2->setText(pi2.name);
        sb2->setMinimum(pi2.min.value_or(min));
        sb2->setMaximum(pi2.max.value_or(max));
        sb2->setDecimals(pi2.precision);
        sb2->setSingleStep(pi2.step);
        sb2->setValue(pi2.value.value());
        label2->setVisible(true);
        sb2->setVisible(true);
    }
    if (n > 2) {
        ParameterInfo pi3 = pv.paramList[2];
        label3->setText(pi3.name);
        sb3->setMinimum(pi3.min.value_or(min));
        sb3->setMaximum(pi3.max.value_or(max));
        sb3->setDecimals(pi3.precision);
        sb3->setSingleStep(pi3.step);
        sb3->setValue(pi3.value.value());
        label3->setVisible(true);
        sb3->setVisible(true);
    }
}

void GenericDistributionDialog::setDistribution(DistributionID id)
{
    // For selected distributions, set more appropriate defaults.

    switch (id) {
        case DistributionID::UniformInt:
            currentDist = Distribution::UniformInt(0, 10);
            break;
        case DistributionID::UniformReal:
            currentDist = Distribution::UniformReal(0, 10);
            break;
        case DistributionID::Binomial:
            currentDist = Distribution::Binomial(10, 0.5);
            break;
        case DistributionID::Geometric:
            currentDist = Distribution::Geometric();
            break;
        case DistributionID::NegativeBinomial:
            currentDist = Distribution::NegativeBinomial(1, 0.5);
            break;
        case DistributionID::Poisson:
            currentDist = Distribution::Poisson(10);
            break;
        case DistributionID::Exponential:
            currentDist = Distribution::Exponential();
            break;
        case DistributionID::Gamma:
            currentDist = Distribution::Gamma();
            break;
        case DistributionID::Weibull:
            currentDist = Distribution::Weibull(1, 1);
            break;
        case DistributionID::ExtremeValue:
            currentDist = Distribution::ExtremeValue();
            break;
        case DistributionID::Beta:
            currentDist = Distribution::Beta(0.5, 2);
            break;
        case DistributionID::Laplace:
            currentDist = Distribution::Laplace();
            break;
        case DistributionID::Normal:
            currentDist = Distribution::Normal();
            break;
        case DistributionID::Lognormal:
            currentDist = Distribution::Lognormal();
            break;
        case DistributionID::ChiSquared:
            currentDist = Distribution::ChiSquared(2);
            break;
        case DistributionID::NCChiSquared:
            currentDist = Distribution::NCChiSquared();
            break;
        case DistributionID::Cauchy:
            currentDist = Distribution::Cauchy();
            break;
        case DistributionID::FisherF:
            currentDist = Distribution::FisherF(2, 1);
            break;
        case DistributionID::StudentT:
            currentDist = Distribution::StudentT();
            break;
        case DistributionID::Discrete:
            currentDist = Distribution::Discrete();
            break;
        case DistributionID::PiecewiseConstant:
            currentDist = Distribution::PiecewiseConstant();
            break;
        case DistributionID::PiecewiseLinear:
            currentDist = Distribution::PiecewiseLinear();
            break;
        case DistributionID::Triangle:
            currentDist = Distribution::Triangle(0.5, 1.0, 1.5);
            break;
    }
}

GenericDistribution GenericDistributionDialog::getDistribution() const
{
    return currentDist;
}

void GenericDistributionDialog::updatePlot()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Distribution");

    double step;
    QVector<QPointF> samples;
    QVector<double> xvec;
    QVector<double> yvec;

    // Set curve styles
    DistributionInfo info = distributionMap.value(currentID);
    switch (info.type) {
        case DT::Continuous:
            step = 0.01;
            pdfPlotCurve->setPen(Qt::blue, 1.5);
            pdfPlotCurve->setStyle(QwtPlotCurve::Lines);
            break;
        case DT::Discrete:
            step = 1;
            pdfPlotCurve->setPen(Qt::blue, 2);
            pdfPlotCurve->setStyle(QwtPlotCurve::Sticks);
            break;
        case DT::Step:
            step = 0.01;
            pdfPlotCurve->setPen(Qt::blue, 1.5);
            pdfPlotCurve->setStyle(QwtPlotCurve::Steps);
            break;
    }

    // Reset the plot
    pdfPlotCurveData->setSamples(samples);
    pdfPlotCurve->setData(pdfPlotCurveData);
    pdfPlot->replot();

    // Estimate X-axis limits
    PdfLimitsVisitor pv;
    boost::apply_visitor(pv, currentDist);

    // Populate independent variable vector
    for (double x = pv.x0; x < pv.x1; x += step)
        xvec.push_back(x);
    xvec.push_back(pv.x1); // last value

    // Evaluate the PDF at each value
    try {
        currentDist.pdf(xvec, yvec);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(warning) << e.what();
        return;
    }

    if (xvec.size() != yvec.size())
        return;

    for (int i=0; i < xvec.size(); ++i) {
        QPointF point(xvec[i], yvec[i]);
        samples.push_back(point);
    }

    pdfPlotCurveData->setSamples(samples);
    pdfPlotCurve->setData(pdfPlotCurveData);
    pdfPlot->replot();
}

void GenericDistributionDialog::onValueChanged(double)
{
    if (static_cast<int>(currentID) < 0)
        return;

    DistributionID id = static_cast<DistributionID>(currentID);

    double a = sb1->value();
    double b = sb2->value();
    double c = sb3->value();

    switch (id) {
        case DistributionID::UniformInt:
            currentDist = Distribution::UniformInt(a, b);
            break;
        case DistributionID::UniformReal:
            currentDist = Distribution::UniformReal(a, b);
            break;
        case DistributionID::Binomial:
            currentDist = Distribution::Binomial(a, b);
            break;
        case DistributionID::Geometric:
            currentDist = Distribution::Geometric(a);
            break;
        case DistributionID::NegativeBinomial:
            currentDist = Distribution::NegativeBinomial(a, b);
            break;
        case DistributionID::Poisson:
            currentDist = Distribution::Poisson(a);
            break;
        case DistributionID::Exponential:
            currentDist = Distribution::Exponential(a);
            break;
        case DistributionID::Gamma:
            currentDist = Distribution::Gamma(a, b);
            break;
        case DistributionID::Weibull:
            currentDist = Distribution::Weibull(a, b);
            break;
        case DistributionID::ExtremeValue:
            currentDist = Distribution::ExtremeValue(a, b);
            break;
        case DistributionID::Beta:
            currentDist = Distribution::Beta(a, b);
            break;
        case DistributionID::Laplace:
            currentDist = Distribution::Laplace(a, b);
            break;
        case DistributionID::Normal:
            currentDist = Distribution::Normal(a, b);
            break;
        case DistributionID::Lognormal:
            currentDist = Distribution::Lognormal(a, b);
            break;
        case DistributionID::ChiSquared:
            currentDist = Distribution::ChiSquared(a);
            break;
        case DistributionID::NCChiSquared:
            currentDist = Distribution::NCChiSquared(a, b);
            break;
        case DistributionID::Cauchy:
            currentDist = Distribution::Cauchy(a, b);
            break;
        case DistributionID::FisherF:
            currentDist = Distribution::FisherF(a, b);
            break;
        case DistributionID::StudentT:
            currentDist = Distribution::StudentT(a);
            break;
        case DistributionID::Discrete:
            currentDist = Distribution::Discrete();
            break;
        case DistributionID::PiecewiseConstant:
            currentDist = Distribution::PiecewiseConstant();
            break;
        case DistributionID::PiecewiseLinear:
            currentDist = Distribution::PiecewiseLinear();
            break;
        case DistributionID::Triangle:
            currentDist = Distribution::Triangle(a, b, c);
            break;
    }

    updatePlot();
}
