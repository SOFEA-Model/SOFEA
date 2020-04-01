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
#include "AppStyle.h"

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

using DT = DistributionType;
using DC = DistributionCategory;

// FIXME: refactor lookup tables

const QMap<DistributionCategory, QString> categoryMap {
    { DC::Uniform,       "Uniform Distributions"       },
    { DC::Bernoulli,     "Bernoulli Distributions"     },
    { DC::Poisson,       "Poisson Distributions"       },
    { DC::Normal,        "Normal Distributions"        },
    { DC::Sampling,      "Sampling Distributions"      },
    { DC::Miscellaneous, "Miscellaneous Distributions" }
};

// Reverse Sort
const QMultiMap<DistributionCategory, GenericDistribution::DistributionID> categoryToIDs {
    { DC::Uniform,        GenericDistribution::ID_UniformReal       },
    { DC::Uniform,        GenericDistribution::ID_UniformInt        },
    { DC::Bernoulli,      GenericDistribution::ID_NegativeBinomial  },
    { DC::Bernoulli,      GenericDistribution::ID_Geometric         },
    { DC::Bernoulli,      GenericDistribution::ID_Binomial          },
    { DC::Poisson,        GenericDistribution::ID_Laplace           },
    { DC::Poisson,        GenericDistribution::ID_Beta              },
    { DC::Poisson,        GenericDistribution::ID_ExtremeValue      },
    { DC::Poisson,        GenericDistribution::ID_Weibull           },
    { DC::Poisson,        GenericDistribution::ID_Gamma             },
    { DC::Poisson,        GenericDistribution::ID_Exponential       },
    { DC::Poisson,        GenericDistribution::ID_Poisson           },
    { DC::Normal,         GenericDistribution::ID_StudentT          },
    { DC::Normal,         GenericDistribution::ID_FisherF           },
    { DC::Normal,         GenericDistribution::ID_Cauchy            },
    { DC::Normal,         GenericDistribution::ID_NCChiSquared      },
    { DC::Normal,         GenericDistribution::ID_ChiSquared        },
    { DC::Normal,         GenericDistribution::ID_Lognormal         },
    { DC::Normal,         GenericDistribution::ID_Normal            },
    { DC::Sampling,       GenericDistribution::ID_PiecewiseLinear   },
    { DC::Sampling,       GenericDistribution::ID_PiecewiseConstant },
    { DC::Sampling,       GenericDistribution::ID_Discrete          },
    { DC::Miscellaneous,  GenericDistribution::ID_Triangle          }
};

struct DistributionInfo
{
    DistributionCategory category;
    DistributionType type;
    QString fullName;
};

const QMap<GenericDistribution::DistributionID, DistributionInfo> distributionMap {
    { GenericDistribution::ID_UniformReal,        { DC::Uniform,        DT::Continuous,  "Uniform Distribution (Real)"         }},
    { GenericDistribution::ID_UniformInt,         { DC::Uniform,        DT::Discrete,    "Uniform Distribution (Int)"          }},
    { GenericDistribution::ID_NegativeBinomial,   { DC::Bernoulli,      DT::Discrete,    "Negative Binomial Distribution"      }},
    { GenericDistribution::ID_Geometric,          { DC::Bernoulli,      DT::Discrete,    "Geometric Distribution"              }},
    { GenericDistribution::ID_Binomial,           { DC::Bernoulli,      DT::Discrete,    "Binomial Distribution"               }},
    { GenericDistribution::ID_Laplace,            { DC::Poisson,        DT::Continuous,  "Laplace Distribution"                }},
    { GenericDistribution::ID_Beta,               { DC::Poisson,        DT::Continuous,  "Beta Distribution"                   }},
    { GenericDistribution::ID_ExtremeValue,       { DC::Poisson,        DT::Continuous,  "Extreme Value Distribution"          }},
    { GenericDistribution::ID_Weibull,            { DC::Poisson,        DT::Continuous,  "Weibull Distribution"                }},
    { GenericDistribution::ID_Gamma,              { DC::Poisson,        DT::Continuous,  "Gamma (Erlang) Distribution"         }},
    { GenericDistribution::ID_Exponential,        { DC::Poisson,        DT::Continuous,  "Exponential Distribution"            }},
    { GenericDistribution::ID_Poisson,            { DC::Poisson,        DT::Discrete,    "Poisson Distribution"                }},
    { GenericDistribution::ID_StudentT,           { DC::Normal,         DT::Continuous,  "Student's t Distribution"            }},
    { GenericDistribution::ID_FisherF,            { DC::Normal,         DT::Continuous,  "F Distribution"                      }},
    { GenericDistribution::ID_Cauchy,             { DC::Normal,         DT::Continuous,  "Cauchy-Lorentz Distribution"         }},
    { GenericDistribution::ID_NCChiSquared,       { DC::Normal,         DT::Continuous,  "Noncentral Chi-Squared Distribution" }},
    { GenericDistribution::ID_ChiSquared,         { DC::Normal,         DT::Continuous,  "Chi Squared Distribution"            }},
    { GenericDistribution::ID_Lognormal,          { DC::Normal,         DT::Continuous,  "Log Normal Distribution"             }},
    { GenericDistribution::ID_Normal,             { DC::Normal,         DT::Continuous,  "Normal (Gaussian) Distribution"      }},
    { GenericDistribution::ID_PiecewiseLinear,    { DC::Sampling,       DT::Continuous,  "Piecewise Linear Distribution"       }},
    { GenericDistribution::ID_PiecewiseConstant,  { DC::Sampling,       DT::Step,        "Piecewise Constant Distribution"     }},
    { GenericDistribution::ID_Discrete,           { DC::Sampling,       DT::Discrete,    "Discrete Distribution"               }},
    { GenericDistribution::ID_Triangle,           { DC::Miscellaneous,  DT::Continuous,  "Triangular Distribution"             }}
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
const QMultiMap<GenericDistribution::DistributionID, ParameterInfo> parameterMap = {
//    Distribution                                 Name                               Value       Minimum       Maximum      Decimals    Step
//    -----------------------------------------    ------------------------     -----------   -----------   -----------   -----------    ----
    { GenericDistribution::ID_UniformInt,        { "Upper: ",                   boost::none,  boost::none,  boost::none,            0,      1 }},
    { GenericDistribution::ID_UniformInt,        { "Lower: ",                   boost::none,  boost::none,  boost::none,            0,      1 }},
    { GenericDistribution::ID_UniformReal,       { "Upper: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_UniformReal,       { "Lower: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Binomial,          { "Probability of success: ",  boost::none,     0.000001,        1.000,            6,   0.01 }},
    { GenericDistribution::ID_Binomial,          { "Trials: ",                  boost::none,            1,  boost::none,            0,      1 }},
    { GenericDistribution::ID_Geometric,         { "Probability: ",             boost::none,     0.000001,        0.999,            6,   0.01 }},
    { GenericDistribution::ID_NegativeBinomial,  { "Probability of success: ",  boost::none,     0.000001,        0.999,            6,   0.01 }},
    { GenericDistribution::ID_NegativeBinomial,  { "Successes: ",               boost::none,            1,  boost::none,            0,      1 }},
    { GenericDistribution::ID_Beta,              { "Beta: ",                    boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Beta,              { "Alpha: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Exponential,       { "Rate: ",                    boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_ExtremeValue,      { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_ExtremeValue,      { "Location: ",                boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Gamma,             { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Gamma,             { "Shape: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Laplace,           { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Laplace,           { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Poisson,           { "Mean: ",                    boost::none,     0.000001,  boost::none,            6,      1 }},
    { GenericDistribution::ID_Weibull,           { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Weibull,           { "Shape: ",                   boost::none,            1,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Cauchy,            { "Scale: ",                   boost::none,            0,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Cauchy,            { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_ChiSquared,        { "Degrees of freedom: ",      boost::none,            2,         1000,            0,      1 }},
    { GenericDistribution::ID_NCChiSquared,      { "Non-centrality: ",          boost::none,     0.000001,         1000,            6,   0.01 }},
    { GenericDistribution::ID_NCChiSquared,      { "Degrees of freedom: ",      boost::none,            1,         1000,            0,      1 }},
    { GenericDistribution::ID_FisherF,           { "Degrees of freedom 2: ",    boost::none,            1,          100,            0,      1 }},
    { GenericDistribution::ID_FisherF,           { "Degrees of freedom 1: ",    boost::none,            2,           20,            0,      1 }},
    { GenericDistribution::ID_Lognormal,         { "Scale: ",                   boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Lognormal,         { "Location: ",                boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Normal,            { "Standard deviation: ",      boost::none,     0.000001,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Normal,            { "Mean: ",                    boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_StudentT,          { "Degrees of freedom: ",      boost::none,            1,          100,            0,      1 }},
    { GenericDistribution::ID_Triangle,          { "Upper: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Triangle,          { "Mode: ",                    boost::none,  boost::none,  boost::none,            6,   0.01 }},
    { GenericDistribution::ID_Triangle,          { "Lower: ",                   boost::none,  boost::none,  boost::none,            6,   0.01 }}
};

// Remaps parameters of the Boost.Random distribution to ParameterInfo.
// TODO: store vector of functions? e.g.
// std::vector<boost::function<void>> functions;
// functions.push_back(&a));
// functions.push_back(&b));

struct ParamVisitor : public boost::static_visitor<>
{
    void operator()(const Distribution::UniformReal& d) {
        id = GenericDistribution::ID_UniformReal;
        paramList = parameterMap.values(GenericDistribution::ID_UniformReal);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::UniformInt& d) {
        id = GenericDistribution::ID_UniformInt;
        paramList = parameterMap.values(GenericDistribution::ID_UniformInt);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::Binomial& d) {
        id = GenericDistribution::ID_Binomial;
        paramList = parameterMap.values(GenericDistribution::ID_Binomial);
        paramList[0].value = d.t();
        paramList[1].value = d.p();
    }
    void operator()(const Distribution::Geometric& d) {
        id = GenericDistribution::ID_Geometric;
        paramList = parameterMap.values(GenericDistribution::ID_Geometric);
        paramList[0].value = d.p();
    }
    void operator()(const Distribution::NegativeBinomial& d) {
        id = GenericDistribution::ID_NegativeBinomial;
        paramList = parameterMap.values(GenericDistribution::ID_NegativeBinomial);
        paramList[0].value = d.k();
        paramList[1].value = d.p();
    }
    void operator()(const Distribution::Poisson& d) {
        id = GenericDistribution::ID_Poisson;
        paramList = parameterMap.values(GenericDistribution::ID_Poisson);
        paramList[0].value = d.mean();
    }
    void operator()(const Distribution::Exponential& d) {
        id = GenericDistribution::ID_Exponential;
        paramList = parameterMap.values(GenericDistribution::ID_Exponential);
        paramList[0].value = d.lambda();
    }
    void operator()(const Distribution::Gamma& d) {
        id = GenericDistribution::ID_Gamma;
        paramList = parameterMap.values(GenericDistribution::ID_Gamma);
        paramList[0].value = d.alpha();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Weibull& d) {
        id = GenericDistribution::ID_Weibull;
        paramList = parameterMap.values(GenericDistribution::ID_Weibull);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::ExtremeValue& d) {
        id = GenericDistribution::ID_ExtremeValue;
        paramList = parameterMap.values(GenericDistribution::ID_ExtremeValue);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::Beta& d) {
        id = GenericDistribution::ID_Beta;
        paramList = parameterMap.values(GenericDistribution::ID_Beta);
        paramList[0].value = d.alpha();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Laplace& d) {
        id = GenericDistribution::ID_Laplace;
        paramList = parameterMap.values(GenericDistribution::ID_Laplace);
        paramList[0].value = d.mean();
        paramList[1].value = d.beta();
    }
    void operator()(const Distribution::Normal& d) {
        id = GenericDistribution::ID_Normal;
        paramList = parameterMap.values(GenericDistribution::ID_Normal);
        paramList[0].value = d.mean();
        paramList[1].value = d.sigma();
    }
    void operator()(const Distribution::Lognormal& d) {
        id = GenericDistribution::ID_Lognormal;
        paramList = parameterMap.values(GenericDistribution::ID_Lognormal);
        paramList[0].value = d.m();
        paramList[1].value = d.s();
    }
    void operator()(const Distribution::ChiSquared& d) {
        id = GenericDistribution::ID_ChiSquared;
        paramList = parameterMap.values(GenericDistribution::ID_ChiSquared);
        paramList[0].value = d.n();
    }
    void operator()(const Distribution::NCChiSquared& d) {
        id = GenericDistribution::ID_NCChiSquared;
        paramList = parameterMap.values(GenericDistribution::ID_NCChiSquared);
        paramList[0].value = d.k();
        paramList[1].value = d.lambda();
    }
    void operator()(const Distribution::Cauchy& d) {
        id = GenericDistribution::ID_Cauchy;
        paramList = parameterMap.values(GenericDistribution::ID_Cauchy);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
    }
    void operator()(const Distribution::FisherF& d) {
        id = GenericDistribution::ID_FisherF;
        paramList = parameterMap.values(GenericDistribution::ID_FisherF);
        paramList[0].value = d.m();
        paramList[1].value = d.n();
    }
    void operator()(const Distribution::StudentT& d) {
        id = GenericDistribution::ID_StudentT;
        paramList = parameterMap.values(GenericDistribution::ID_StudentT);
        paramList[0].value = d.n();
    }
    void operator()(const Distribution::Discrete& d) {
        id = GenericDistribution::ID_Discrete;
    }
    void operator()(const Distribution::PiecewiseConstant& d) {
        id = GenericDistribution::ID_PiecewiseConstant;
    }
    void operator()(const Distribution::PiecewiseLinear& d) {
        id = GenericDistribution::ID_PiecewiseLinear;
    }
    void operator()(const Distribution::Triangle& d) {
        id = GenericDistribution::ID_Triangle;
        paramList = parameterMap.values(GenericDistribution::ID_Triangle);
        paramList[0].value = d.a();
        paramList[1].value = d.b();
        paramList[2].value = d.c();
    }
    void operator()(const Distribution::Constant& d) {}

    GenericDistribution::DistributionID id;
    QList<ParameterInfo> paramList;
};

// Sets reasonable default limits for the PDF plot, based on current parameters.

struct PdfLimitsVisitor : public boost::static_visitor<>
{
    void operator()(const Distribution::UniformReal& d) {
        x0 = d.a() - (d.b() - d.a());
        x1 = d.b() + (d.b() - d.a());
    }
    void operator()(const Distribution::UniformInt& d) {
        x0 = d.a() - (d.b() - d.a());
        x1 = d.b() + (d.b() - d.a());
    }
    void operator()(const Distribution::Binomial& d) {
        x0 = 0;
        x1 = d.t();
    }
    void operator()(const Distribution::Geometric& d) {
        x0 = 1;
        x1 = 10;
    }
    void operator()(const Distribution::NegativeBinomial& d) {
        x0 = 0;
        x1 = 25;
    }
    void operator()(const Distribution::Poisson& d) {
        x0 = 0;
        x1 = 2 * d.mean();
    }
    void operator()(const Distribution::Exponential& d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::Gamma& d) {
        x0 = 0;
        x1 = d.alpha() * 2;
    }
    void operator()(const Distribution::Weibull& d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::ExtremeValue& d) {
        x0 = d.a() - (4 * d.b());
        x1 = d.a() + (6 * d.b());
    }
    void operator()(const Distribution::Beta& d) {
        x0 = 0.01;
        x1 = 0.99;
    }
    void operator()(const Distribution::Laplace& d) {
        x0 = d.mean()- (4 * d.beta());
        x1 = d.mean() + (4 * d.beta());
    }
    void operator()(const Distribution::Normal& d) {
        x0 = d.mean() - 5;
        x1 = d.mean() + 5;
    }
    void operator()(const Distribution::Lognormal& d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::ChiSquared& d) {
        x0 = 0;
        x1 = 8;
    }
    void operator()(const Distribution::NCChiSquared& d) {
        x0 = 0;
        x1 = 8;
    }
    void operator()(const Distribution::Cauchy& d) {
        x0 = d.a() - 5;
        x1 = d.a() + 5;
    }
    void operator()(const Distribution::FisherF& d) {
        x0 = 0;
        x1 = 5;
    }
    void operator()(const Distribution::StudentT& d) {
        x0 = -5;
        x1 = 5;
    }
    void operator()(const Distribution::Discrete& d) {}
    void operator()(const Distribution::PiecewiseConstant& d) {}
    void operator()(const Distribution::PiecewiseLinear& d) {}
    void operator()(const Distribution::Triangle& d) {
        x0 = d.a();
        x1 = d.c();
    }
    void operator()(const Distribution::Constant& d) {}

    double x0;
    double x1;
};

GenericDistributionDialog::GenericDistributionDialog(const GenericDistribution& d, QWidget *parent)
    : QDialog(parent), currentDist(d)
{
    setWindowTitle(tr("Define Distribution"));
    setWindowFlag(Qt::Tool);
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_LineEditFunctionIcon)));

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

    pdfPlotCurveData = new QwtPointSeriesData;
    pdfPlotCurve = new QwtPlotCurve;
    pdfPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    pdfPlotCurve->attach(pdfPlot);

    QColor baseColor = QWidget::palette().base().color();

    QFormLayout *controlsLayout = new QFormLayout;
    controlsLayout->addRow(label1, sb1);
    controlsLayout->addRow(label2, sb2);
    controlsLayout->addRow(label3, sb3);
    QFrame *controlsFrame = new QFrame;
    controlsFrame->setLayout(controlsLayout);
    controlsFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QPalette controlsFramePalette = controlsFrame->palette();
    controlsFramePalette.setColor(QPalette::Window, baseColor);
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
    plotFramePalette.setColor(QPalette::Window, baseColor);
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
            currentID = static_cast<GenericDistribution::DistributionID>(id);
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

    sb1->blockSignals(true);
    sb2->blockSignals(true);
    sb3->blockSignals(true);

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
        sb1->blockSignals(false);
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
        sb2->blockSignals(false);
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
        sb3->blockSignals(false);
    }
}

void GenericDistributionDialog::setDistribution(GenericDistribution::DistributionID id)
{
    // For selected distributions, set more appropriate defaults.

    switch (id) {
        case GenericDistribution::ID_UniformInt:
            currentDist = Distribution::UniformInt(0, 10);
            break;
        case GenericDistribution::ID_UniformReal:
            currentDist = Distribution::UniformReal(0, 10);
            break;
        case GenericDistribution::ID_Binomial:
            currentDist = Distribution::Binomial(10, 0.5);
            break;
        case GenericDistribution::ID_Geometric:
            currentDist = Distribution::Geometric();
            break;
        case GenericDistribution::ID_NegativeBinomial:
            currentDist = Distribution::NegativeBinomial(10, 0.5);
            break;
        case GenericDistribution::ID_Poisson:
            currentDist = Distribution::Poisson(10);
            break;
        case GenericDistribution::ID_Exponential:
            currentDist = Distribution::Exponential();
            break;
        case GenericDistribution::ID_Gamma:
            currentDist = Distribution::Gamma(5, 1);
            break;
        case GenericDistribution::ID_Weibull:
            currentDist = Distribution::Weibull(1, 1);
            break;
        case GenericDistribution::ID_ExtremeValue:
            currentDist = Distribution::ExtremeValue();
            break;
        case GenericDistribution::ID_Beta:
            currentDist = Distribution::Beta(0.5, 2);
            break;
        case GenericDistribution::ID_Laplace:
            currentDist = Distribution::Laplace();
            break;
        case GenericDistribution::ID_Normal:
            currentDist = Distribution::Normal(0, 1);
            break;
        case GenericDistribution::ID_Lognormal:
            currentDist = Distribution::Lognormal();
            break;
        case GenericDistribution::ID_ChiSquared:
            currentDist = Distribution::ChiSquared(2);
            break;
        case GenericDistribution::ID_NCChiSquared:
            currentDist = Distribution::NCChiSquared();
            break;
        case GenericDistribution::ID_Cauchy:
            currentDist = Distribution::Cauchy();
            break;
        case GenericDistribution::ID_FisherF:
            currentDist = Distribution::FisherF(2, 1);
            break;
        case GenericDistribution::ID_StudentT:
            currentDist = Distribution::StudentT();
            break;
        case GenericDistribution::ID_Discrete:
            currentDist = Distribution::Discrete();
            break;
        case GenericDistribution::ID_PiecewiseConstant:
            currentDist = Distribution::PiecewiseConstant();
            break;
        case GenericDistribution::ID_PiecewiseLinear:
            currentDist = Distribution::PiecewiseLinear();
            break;
        case GenericDistribution::ID_Triangle:
            currentDist = Distribution::Triangle(-1, 0, 1);
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

    auto id = static_cast<GenericDistribution::DistributionID>(currentID);

    double a = sb1->value();
    double b = sb2->value();
    double c = sb3->value();

    switch (id) {
        case GenericDistribution::ID_UniformInt:
            currentDist = Distribution::UniformInt(a, b);
            break;
        case GenericDistribution::ID_UniformReal:
            currentDist = Distribution::UniformReal(a, b);
            break;
        case GenericDistribution::ID_Binomial:
            currentDist = Distribution::Binomial(a, b);
            break;
        case GenericDistribution::ID_Geometric:
            currentDist = Distribution::Geometric(a);
            break;
        case GenericDistribution::ID_NegativeBinomial:
            currentDist = Distribution::NegativeBinomial(a, b);
            break;
        case GenericDistribution::ID_Poisson:
            currentDist = Distribution::Poisson(a);
            break;
        case GenericDistribution::ID_Exponential:
            currentDist = Distribution::Exponential(a);
            break;
        case GenericDistribution::ID_Gamma:
            currentDist = Distribution::Gamma(a, b);
            break;
        case GenericDistribution::ID_Weibull:
            currentDist = Distribution::Weibull(a, b);
            break;
        case GenericDistribution::ID_ExtremeValue:
            currentDist = Distribution::ExtremeValue(a, b);
            break;
        case GenericDistribution::ID_Beta:
            currentDist = Distribution::Beta(a, b);
            break;
        case GenericDistribution::ID_Laplace:
            currentDist = Distribution::Laplace(a, b);
            break;
        case GenericDistribution::ID_Normal:
            currentDist = Distribution::Normal(a, b);
            break;
        case GenericDistribution::ID_Lognormal:
            currentDist = Distribution::Lognormal(a, b);
            break;
        case GenericDistribution::ID_ChiSquared:
            currentDist = Distribution::ChiSquared(a);
            break;
        case GenericDistribution::ID_NCChiSquared:
            currentDist = Distribution::NCChiSquared(a, b);
            break;
        case GenericDistribution::ID_Cauchy:
            currentDist = Distribution::Cauchy(a, b);
            break;
        case GenericDistribution::ID_FisherF:
            currentDist = Distribution::FisherF(a, b);
            break;
        case GenericDistribution::ID_StudentT:
            currentDist = Distribution::StudentT(a);
            break;
        case GenericDistribution::ID_Discrete:
            currentDist = Distribution::Discrete();
            break;
        case GenericDistribution::ID_PiecewiseConstant:
            currentDist = Distribution::PiecewiseConstant();
            break;
        case GenericDistribution::ID_PiecewiseLinear:
            currentDist = Distribution::PiecewiseLinear();
            break;
        case GenericDistribution::ID_Triangle:
            currentDist = Distribution::Triangle(a, b, c);
            break;
    }

    updatePlot();
}
