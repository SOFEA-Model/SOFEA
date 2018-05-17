#include <QtWidgets>

#include "SourceEditor.h"

SourceEditor::SourceEditor(QWidget *parent) : QWidget(parent)
{
    areaEditor = new AreaSourceEditor;
    areaCircEditor = new AreaCircSourceEditor;
    areaPolyEditor = new AreaPolySourceEditor;

    stack = new QStackedLayout;
    stack->setContentsMargins(0,0,0,0);
    stack->addWidget(areaEditor);
    stack->addWidget(areaCircEditor);
    stack->addWidget(areaPolyEditor);

    setLayout(stack);
}

void SourceEditor::setSource(Source *s)
{
    switch (s->getType()) {
        case SourceType::AREA: {
            AreaSource *sa = static_cast<AreaSource*>(s);
            areaEditor->setSource(sa);
            stack->setCurrentIndex(0);
            break;
        }
        case SourceType::AREACIRC: {
            AreaCircSource *sc = static_cast<AreaCircSource*>(s);
            areaCircEditor->setSource(sc);
            stack->setCurrentIndex(1);
            break;
        }
        case SourceType::AREAPOLY: {
            AreaPolySource *sp = static_cast<AreaPolySource*>(s);
            areaPolyEditor->setSource(sp);
            stack->setCurrentIndex(2);
            break;
        }
        default: {
            break;
        }
    }
}

/****************************************************************************
** AREA
****************************************************************************/

AreaSourceEditor::AreaSourceEditor(QWidget *parent) : QWidget(parent)
{
    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbXCoord->setPrefix("X=");

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbYCoord->setPrefix("Y=");

    sbXInit = new QDoubleSpinBox();
    sbXInit->setRange(0, 10000);
    sbXInit->setDecimals(2);
    sbXInit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbXInit->setPrefix("X=");

    sbYInit = new QDoubleSpinBox();
    sbYInit->setRange(0, 10000);
    sbYInit->setDecimals(2);
    sbYInit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbYInit->setPrefix("Y=");

    sbAngle = new QDoubleSpinBox;
    sbAngle->setMinimum(0);
    sbAngle->setMaximum(359.9);
    sbAngle->setSingleStep(1);
    sbAngle->setDecimals(1);
    sbAngle->setWrapping(true);

    // Connections
    connect(sbXCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaSourceEditor::onCoordinatesChanged);

    connect(sbYCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaSourceEditor::onCoordinatesChanged);

    connect(sbXInit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaSourceEditor::onCoordinatesChanged);

    connect(sbYInit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaSourceEditor::onCoordinatesChanged);

    connect(sbAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaSourceEditor::onCoordinatesChanged);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0,0,0,0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Coordinates (m):")), 0, 0);
    layout1->addWidget(sbXCoord, 0, 1);
    layout1->addWidget(sbYCoord, 0, 2);
    layout1->addWidget(new QLabel(tr("Dimensions (m):")), 1, 0);
    layout1->addWidget(sbXInit, 1, 1);
    layout1->addWidget(sbYInit, 1, 2);
    layout1->addWidget(new QLabel(QLatin1String("Angle (\u00b0CW):")), 2, 0);
    layout1->addWidget(sbAngle, 2, 1, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

void AreaSourceEditor::setSource(AreaSource *s)
{
    sbXCoord->setValue(s->xs);
    sbYCoord->setValue(s->ys);
    sbXInit->setValue(s->xinit);
    sbYInit->setValue(s->yinit);
    sbAngle->setValue(s->angle);

    sPtr = s;
}

void AreaSourceEditor::onCoordinatesChanged(double)
{
    if (sPtr == nullptr)
        return;

    sPtr->xs = sbXCoord->value();
    sPtr->ys = sbYCoord->value();
    sPtr->xinit = sbXInit->value();
    sPtr->yinit = sbYInit->value();
    sPtr->angle = sbAngle->value();
}

/****************************************************************************
** AREACIRC
****************************************************************************/

AreaCircSourceEditor::AreaCircSourceEditor(QWidget *parent) : QWidget(parent)
{
    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbXCoord->setPrefix("X=");

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbYCoord->setPrefix("Y=");

    sbRadius = new QDoubleSpinBox();
    sbRadius->setRange(0, 10000);
    sbRadius->setDecimals(2);
    sbRadius->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbVertexCount = new QSpinBox;
    sbVertexCount->setRange(3, 100);
    sbVertexCount->setButtonSymbols(QAbstractSpinBox::NoButtons);

    // Connections
    connect(sbXCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaCircSourceEditor::onCoordinatesChanged);

    connect(sbYCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaCircSourceEditor::onCoordinatesChanged);

    connect(sbRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaCircSourceEditor::onCoordinatesChanged);

    connect(sbVertexCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AreaCircSourceEditor::onCoordinatesChanged);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0,0,0,0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Coordinates (m):")), 0, 0);
    layout1->addWidget(sbXCoord, 0, 1);
    layout1->addWidget(sbYCoord, 0, 2);
    layout1->addWidget(new QLabel(tr("Radius (m):")), 1, 0);
    layout1->addWidget(sbRadius, 1, 1, 1, 2);
    layout1->addWidget(new QLabel(tr("Vertex count:")), 2, 0);
    layout1->addWidget(sbVertexCount, 2, 1, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

void AreaCircSourceEditor::setSource(AreaCircSource *s)
{
    sbXCoord->setValue(s->xs);
    sbYCoord->setValue(s->ys);
    sbRadius->setValue(s->radius);
    sbVertexCount->setValue(s->nverts);

    sPtr = s;
}

void AreaCircSourceEditor::onCoordinatesChanged(double)
{
    if (sPtr == nullptr)
        return;

    sPtr->xs = sbXCoord->value();
    sPtr->ys = sbYCoord->value();
    sPtr->radius = sbRadius->value();
    sPtr->nverts = sbVertexCount->value();
}

/****************************************************************************
** AREAPOLY
****************************************************************************/

AreaPolySourceEditor::AreaPolySourceEditor(QWidget *parent) : QWidget(parent)
{
    sbVertexCount = new QSpinBox;
    sbVertexCount->setRange(3, 3);
    sbVertexCount->setValue(3);
    sbVertexCount->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbVertex = new QSpinBox;
    sbVertex->setRange(1, 5);
    sbVertex->setWrapping(true);
    sbVertex->setValue(1);

    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbXCoord->setPrefix("X=");

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //sbYCoord->setPrefix("Y=");

    // Connections
    connect(sbVertexCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AreaPolySourceEditor::onVertexCountChanged);

    connect(sbVertex, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AreaPolySourceEditor::onVertexChanged);

    connect(sbXCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaPolySourceEditor::onCoordinatesChanged);

    connect(sbYCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AreaPolySourceEditor::onCoordinatesChanged);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0,0,0,0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Vertex count:")), 0, 0);
    layout1->addWidget(sbVertexCount, 0, 1, 1, 2);
    layout1->addWidget(new QLabel(tr("Selected vertex:")), 1, 0);
    layout1->addWidget(sbVertex, 1, 1, 1, 2);
    layout1->addWidget(new QLabel(tr("Coordinates (m):")), 2, 0);
    layout1->addWidget(sbXCoord, 2, 1);
    layout1->addWidget(sbYCoord, 2, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

void AreaPolySourceEditor::setSource(AreaPolySource *s)
{
    int vertexCount = s->geometry.size();

    const QPointF firstVertex = s->geometry.first();

    if (vertexCount > 25) {
        // Imported source may have more than 25 vertices.
        // Set the maximum vertex count to actual.
        sbVertexCount->setMaximum(vertexCount);
    }
    else {
        // Maximum 25 vertices for a user defined source.
        sbVertexCount->setMaximum(25);
    }

    sbVertexCount->setValue(vertexCount);
    sbVertex->setMaximum(vertexCount);
    sbVertex->setValue(1);
    sbXCoord->setValue(firstVertex.x());
    sbYCoord->setValue(firstVertex.y());

    sPtr = s;
}

void AreaPolySourceEditor::onVertexCountChanged(int i)
{
    if (sPtr == nullptr)
        return;

    sPtr->geometry.resize(i);

    if (i < sbVertex->value()) {
        // Vertex count has decreased. Set controls to last vertex.
        const QPointF lastVertex = sPtr->geometry.at(i - 1);
        sbVertex->setValue(i);
        sbXCoord->setValue(lastVertex.x());
        sbYCoord->setValue(lastVertex.y());
    }

    sbVertex->setMaximum(i);
}

void AreaPolySourceEditor::onVertexChanged(int i)
{
    if (!sPtr)
        return;

    const QPointF currentVertex = sPtr->geometry.at(i - 1);
    sbXCoord->setValue(currentVertex.x());
    sbYCoord->setValue(currentVertex.y());
}

void AreaPolySourceEditor::onCoordinatesChanged(double)
{
    if (!sPtr)
        return;

    int i = sbVertex->value() - 1;
    double x = sbXCoord->value();
    double y = sbYCoord->value();
    QPointF p(x, y);
    sPtr->geometry.replace(i, p);

    // Last vertex must equal first vertex.
    int lastIndex = sPtr->geometry.size() - 1;
    if (i == lastIndex)
        sPtr->geometry.replace(0, p);
    if (i == 0)
        sPtr->geometry.replace(lastIndex, p);
}
