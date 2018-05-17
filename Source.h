#pragma once

#include <cmath>
#include <string>

#include <QDateTime>
#include <QPolygonF>

struct SourceGroup;

enum class SourceType
{
    POINT,
    POINTCAP,
    POINTHOR,
    VOLUME,
    AREA,
    AREAPOLY,
    AREACIRC,
    OPENPIT,
    LINE,
    BUOYLINE
};

struct Source
{
    virtual void setGeometry() = 0;
    virtual std::string toString(int isrc) const = 0;
    virtual SourceType getType() const = 0;
    double area() const;

    std::string srcid;
    double xs = 0;
    double ys = 0;
    double zs = 0;
    QDateTime appStart;
    double appRate = 0;
    double incorpDepth = 0;
    QPolygonF geometry;

    virtual ~Source() {}
    virtual Source* clone() const = 0;
};

// Implement clonable concept for boost::ptr_vector
inline Source* new_clone(const Source& c)
{
    return c.clone();
}

/****************************************************************************
** AREA Source
****************************************************************************/

struct AreaSource : Source
{
    void setGeometry();
    std::string toString(int isrc) const;
    SourceType getType() const {
        return SourceType::AREA;
    }

    double aremis = 1;
    double relhgt = 0;
    double xinit = 100;
    double yinit = 100;
    double angle = 0;
    double szinit = 0;

    AreaSource* clone() const override {
        return new AreaSource(*this);
    }
};

/****************************************************************************
** AREACIRC Source
****************************************************************************/

struct AreaCircSource : Source
{
    void setGeometry();
    std::string toString(int isrc) const;
    SourceType getType() const {
        return SourceType::AREACIRC;
    }

    double aremis = 1;
    double relhgt = 0;
    double radius = 100;
    int nverts = 20;
    double szinit = 0;

    AreaCircSource* clone() const override {
        return new AreaCircSource(*this);
    }
};

/****************************************************************************
** AREAPOLY Source
****************************************************************************/

struct AreaPolySource : Source
{
    void setGeometry();
    std::string toString(int isrc) const;
    SourceType getType() const {
        return SourceType::AREAPOLY;
    }

    double aremis = 1;
    double relhgt = 0;
    double szinit = 0;

    AreaPolySource* clone() const override {
        return new AreaPolySource(*this);
    }
};

