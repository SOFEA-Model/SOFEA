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

#include "BoundingBoxEditor.h"
#include "DoubleLineEdit.h"
#include "GroupBoxFrame.h"
#include "core/Common.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>

#include <QDebug>

BoundingBoxEditor::BoundingBoxEditor(QWidget *parent)
    : QWidget(parent)
{
    using namespace sofea::constants;

    headerLabel = new QLabel(tr("Bounding Box"));

    optionGroup = new QButtonGroup;
    QRadioButton *optionProjected = new QRadioButton(tr("Projected"));
    QRadioButton *optionGeographic = new QRadioButton(tr("Geographic"));
    optionGroup->addButton(optionProjected, Mode::Projected);
    optionGroup->addButton(optionGeographic, Mode::Geographic);
    optionProjected->setChecked(true);

    xminEdit = new DoubleLineEdit(MIN_X_COORDINATE, MAX_X_COORDINATE, X_COORDINATE_PRECISION);
    yminEdit = new DoubleLineEdit(MIN_Y_COORDINATE, MAX_Y_COORDINATE, Y_COORDINATE_PRECISION);
    ymaxEdit = new DoubleLineEdit(MIN_Y_COORDINATE, MAX_Y_COORDINATE, Y_COORDINATE_PRECISION);
    xmaxEdit = new DoubleLineEdit(MIN_X_COORDINATE, MAX_X_COORDINATE, X_COORDINATE_PRECISION);

    // Connections
    connect(optionGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [&](int id) {
        setMode(static_cast<Mode>(id));
    });

    connect(this, &BoundingBoxEditor::valueChanged, [](const QRectF&) {
        qDebug() << "valueChanged";
    });

    connect(xminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMinChanged);
    connect(yminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMinChanged);
    connect(ymaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMaxChanged);
    connect(xmaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMaxChanged);

    // Layout
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(headerLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(optionProjected);
    headerLayout->addWidget(optionGeographic);

    QFormLayout *editorLayoutL = new QFormLayout;
    editorLayoutL->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    editorLayoutL->setSpacing(10);
    editorLayoutL->addRow(tr("West:"), xminEdit);

    QFormLayout *editorLayoutC = new QFormLayout;
    editorLayoutC->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    editorLayoutC->setSpacing(10);
    editorLayoutC->addRow(tr("North:"), ymaxEdit);
    editorLayoutC->addRow(tr("South:"), yminEdit);

    QFormLayout *editorLayoutR = new QFormLayout;
    editorLayoutR->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    editorLayoutR->setSpacing(10);
    editorLayoutR->addRow(tr("East:"), xmaxEdit);

    QHBoxLayout *editorLayout = new QHBoxLayout;
    editorLayout->addLayout(editorLayoutL);
    editorLayout->addLayout(editorLayoutC);
    editorLayout->addLayout(editorLayoutR);

    GroupBoxFrame *editorFrame = new GroupBoxFrame;
    editorFrame->setLayout(editorLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(editorFrame);

    setLayout(mainLayout);
}

void BoundingBoxEditor::setMode(Mode mode)
{
    using namespace sofea::constants;
    using namespace Projection;

    if (mode == mode_)
        return;

    auto pcrs = getComponentCRS(ccrs_, 0);
    auto gcrs = getGeodeticCRS(pcrs);

    auto pipeline = Pipeline(gcrs, pcrs);

    double llx = 0, lly = 0, llz = 0;
    double urx = 0, ury = 0, urz = 0;

    switch (mode) {
    case Projected:
        // Geographic to Projected
        pipeline.forward(xmin(), ymin(), 0, llx, lly, llz); // SW corner
        pipeline.forward(xmax(), ymax(), 0, urx, ury, urz); // NE corner
        xminEdit->setRange(MIN_X_COORDINATE, MAX_X_COORDINATE);
        yminEdit->setRange(MIN_Y_COORDINATE, MAX_Y_COORDINATE);
        ymaxEdit->setRange(MIN_Y_COORDINATE, MAX_Y_COORDINATE);
        xmaxEdit->setRange(MIN_X_COORDINATE, MAX_X_COORDINATE);
        xminEdit->setDecimals(X_COORDINATE_PRECISION);
        yminEdit->setDecimals(Y_COORDINATE_PRECISION);
        ymaxEdit->setDecimals(Y_COORDINATE_PRECISION);
        xmaxEdit->setDecimals(X_COORDINATE_PRECISION);
        break;
    case Geographic:
        // Projected to Geographic
        pipeline.inverse(xmin(), ymin(), 0, llx, lly, llz); // SW corner
        pipeline.inverse(xmax(), ymax(), 0, urx, ury, urz); // NE corner
        xminEdit->setRange(-180.0, 180.0);
        yminEdit->setRange(-90.0, 90.0);
        ymaxEdit->setRange(-90.0, 90.0);
        xmaxEdit->setRange(-180.0, 180.0);
        xminEdit->setDecimals(LONGITUDE_PRECISION);
        yminEdit->setDecimals(LATITUDE_PRECISION);
        ymaxEdit->setDecimals(LATITUDE_PRECISION);
        xmaxEdit->setDecimals(LONGITUDE_PRECISION);
        break;
    default:
        return;
    }

    disconnect(xminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMinChanged);
    disconnect(yminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMinChanged);
    disconnect(ymaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMaxChanged);
    disconnect(xmaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMaxChanged);

    xminEdit->setValue(llx);
    yminEdit->setValue(lly);
    ymaxEdit->setValue(ury);
    xmaxEdit->setValue(urx);

    connect(xminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMinChanged);
    connect(yminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMinChanged);
    connect(ymaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMaxChanged);
    connect(xmaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMaxChanged);

    mode_ = mode;

    emit modeChanged(mode);
    emit valueChanged(value());
}

BoundingBoxEditor::Mode BoundingBoxEditor::mode() const
{
    return mode_;
}

void BoundingBoxEditor::setCompoundCRS(std::shared_ptr<PJ> ccrs)
{
    ccrs_ = ccrs;
}

std::shared_ptr<PJ> BoundingBoxEditor::compoundCRS() const
{
    return ccrs_;
}

void BoundingBoxEditor::setValue(const QRectF& bbox)
{
    disconnect(xminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMinChanged);
    disconnect(yminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMinChanged);
    disconnect(ymaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMaxChanged);
    disconnect(xmaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMaxChanged);

    xminEdit->setValue(bbox.x());
    yminEdit->setValue(bbox.y() - bbox.height());
    xmaxEdit->setValue(bbox.x() + bbox.width());
    ymaxEdit->setValue(bbox.y());

    connect(xminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMinChanged);
    connect(yminEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMinChanged);
    connect(ymaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onYMaxChanged);
    connect(xmaxEdit, &DoubleLineEdit::valueChanged, this, &BoundingBoxEditor::onXMaxChanged);

    emit valueChanged(value());
}

QRectF BoundingBoxEditor::value() const
{
    double x = xminEdit->value();
    double y = ymaxEdit->value();
    double w = x + xmaxEdit->value();
    double h = y - yminEdit->value();

    return QRectF(x, y, w, h);
}

double BoundingBoxEditor::xmin() const
{
    return xminEdit->value();
}

double BoundingBoxEditor::ymin() const
{
    return yminEdit->value();
}

double BoundingBoxEditor::xmax() const
{
    return xmaxEdit->value();
}

double BoundingBoxEditor::ymax() const
{
    return ymaxEdit->value();
}

void BoundingBoxEditor::onXMinChanged(double xmin)
{
    xmaxEdit->setErrorPalette(xmin > xmax());
    emit valueChanged(value());
}

void BoundingBoxEditor::onYMinChanged(double ymin)
{
    ymaxEdit->setErrorPalette(ymin > ymax());
    emit valueChanged(value());
}

void BoundingBoxEditor::onYMaxChanged(double ymax)
{
    yminEdit->setErrorPalette(ymax < ymin());
    emit valueChanged(value());
}

void BoundingBoxEditor::onXMaxChanged(double xmax)
{
    xminEdit->setErrorPalette(xmax < xmin());
    emit valueChanged(value());
}
