#ifndef SOURCEEDITOR_H
#define SOURCEEDITOR_H

#include <vector>

#include <QWidget>

#include "SourceGroup.h"

QT_BEGIN_NAMESPACE
class QDateTimeEdit;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QPolygonF;
class QStackedLayout;
QT_END_NAMESPACE

class AreaSourceEditor;
class AreaCircSourceEditor;
class AreaPolySourceEditor;

class SourceEditor : public QWidget
{
    Q_OBJECT
public:
    SourceEditor(QWidget *parent = nullptr);
    void setSource(Source *s);

private:
    QStackedLayout *stack;
    AreaSourceEditor *areaEditor;
    AreaCircSourceEditor *areaCircEditor;
    AreaPolySourceEditor *areaPolyEditor;
};

/****************************************************************************
** AREA
****************************************************************************/

class AreaSourceEditor : public QWidget
{
    Q_OBJECT
public:
    AreaSourceEditor(QWidget *parent = nullptr);
    void setSource(AreaSource *s);

private slots:
    void onCoordinatesChanged(double);

private:
    AreaSource *sPtr = nullptr;

    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbXInit;
    QDoubleSpinBox *sbYInit;
    QDoubleSpinBox *sbAngle;
};

/****************************************************************************
** AREACIRC
****************************************************************************/

class AreaCircSourceEditor : public QWidget
{
    Q_OBJECT
public:
    AreaCircSourceEditor(QWidget *parent = nullptr);
    void setSource(AreaCircSource *s);

private slots:
    void onCoordinatesChanged(double);

private:
    AreaCircSource *sPtr = nullptr;

    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbRadius;
    QSpinBox *sbVertexCount;
};

/****************************************************************************
** AREAPOLY
****************************************************************************/

class AreaPolySourceEditor : public QWidget
{
    Q_OBJECT
public:
    AreaPolySourceEditor(QWidget *parent = nullptr);
    void setSource(AreaPolySource *s);

private slots:
    void onVertexCountChanged(int i);
    void onVertexChanged(int i);
    void onCoordinatesChanged(double);

private:
    AreaPolySource *sPtr = nullptr;

    QSpinBox *sbVertexCount;
    QSpinBox *sbVertex;
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
};


#endif // SOURCEEDITOR_H
