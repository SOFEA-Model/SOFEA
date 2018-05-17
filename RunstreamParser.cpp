#include "RunstreamParser.h"
#include "Runstream.h"
#include "GeometryOp.h"

#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QPolygonF>
#include <QTransform>

#include <boost/log/trivial.hpp>

// FIXME: allow reading without SO STARTING / SO FINISHED

void parseInternalLocation(QTextStream& is, runstream::source::container& sc)
{
    QString srcid;
    QString st;

    is >> srcid >> st;

    if (is.status() != QTextStream::Ok)
        return; // read failure

    if (st == "AREA") {
        runstream::source::area s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }
    else if (st == "AREAPOLY") {
        runstream::source::areapoly s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }
    else if (st == "AREACIRC") {
        runstream::source::areacirc s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }
}

void parseInternalSrcParam(QTextStream& is, runstream::source::container& sc)
{
    typedef runstream::source::container::nth_index<1>::type indexed_view;

    QString srcid;
    is >> srcid;
    if (is.status() != QTextStream::Ok)
        return; // read failure

    indexed_view& v = sc.get<1>();
    indexed_view::iterator it = v.find(srcid);
    if (it == v.end())
        return; // id not found

    runstream::source::kvp kvp = *it;
    runstream::source::variant sv = kvp.var;

    switch (sv.which())
    {
    case 4: { // AREA
        auto s = boost::get<runstream::source::area>(sv);
        is >> s.aremis >> s.relhgt >> s.xinit;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        if (!is.atEnd())
            is >> s.yinit;
        else
            s.yinit = s.xinit;
        if (!is.atEnd())
            is >> s.angle;
        if (!is.atEnd())
            is >> s.szinit;
        s.complete = true; // terminal parse
        kvp.var = s;
        v.replace(it, kvp);
        break;
    }
    case 5: { // AREAPOLY
        auto s = boost::get<runstream::source::areapoly>(sv);
        is >> s.aremis >> s.relhgt >> s.nverts;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        if (!is.atEnd())
            is >> s.szinit;
        s.complete = false; // non-terminal parse
        kvp.var = s;
        v.replace(it, kvp); // update record
        break;
    }
    case 6: { // AREACIRC
        auto s = boost::get<runstream::source::areacirc>(sv);
        is >> s.aremis >> s.relhgt >> s.radius;
        if (is.status() != QTextStream::Ok)
            return; // read failure
        if (!is.atEnd())
            is >> s.nverts >> s.szinit;
        s.complete = true; // terminal parse
        kvp.var = s;
        v.replace(it, kvp); // update record
        break;
    }
    }
}

void parseInternalAreaVert(QTextStream& is, runstream::source::container& sc)
{
    typedef runstream::source::container::nth_index<1>::type indexed_view;

    QString srcid;
    is >> srcid;
    if (is.status() != QTextStream::Ok)
        return; // read failure

    indexed_view& v = sc.get<1>();
    indexed_view::iterator it = v.find(srcid);
    if (it == v.end())
        return; // id not found

    runstream::source::kvp kvp = *it;
    runstream::source::variant sv = kvp.var;

    if (sv.which() != 5)
        return; // not areapoly type

    auto s = boost::get<runstream::source::areapoly>(sv);
    while (true) {
        double xs, ys;
        is >> xs >> ys;
        if (is.status() != QTextStream::Ok)
            break;
        s.areavert << QPointF(xs, ys);
    }

    if (s.areavert.size() == s.nverts) {
        s.complete = true; // terminal parse
        kvp.var = s;
        v.replace(it, kvp); // update record
    }
}

void RunstreamParser::parseSources(const QString& inputFile, boost::ptr_vector<Source>& sources)
{
    runstream::source::container sc;

    QFile file(inputFile);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // Begin File I/O
    bool block = false;
    while (!file.atEnd())
    {
        QString line = file.readLine();
        QString pathway, keyword;
        line = line.toUpper();
        QTextStream is(&line);

        is >> pathway;

        // Comments
        if (pathway.startsWith("**"))
            continue;

        // Block Delimiters
        if (pathway == "SO")
        {
            is >> keyword;
            if (keyword == "STARTING") {
                block = true;
                continue;
            }
            else if (keyword == "FINISHED") {
                block = false;
                continue;
            }
        }

        // Pathway SOURCE
        if (block)
        {
            if (pathway != "SO") {
                keyword = pathway;
                pathway = "SO";
            }

            if (keyword == "LOCATION") {
                parseInternalLocation(is, sc);
            }
            else if (keyword == "SRCPARAM") {
                parseInternalSrcParam(is, sc);
            }
            else if (keyword == "AREAVERT") {
                parseInternalAreaVert(is, sc);
            }
        }
    }
    // End File I/O

    // Create Sources
    for (auto it = sc.begin(); it != sc.end(); ++it)
    {
        runstream::source::kvp kvp = *it;
        runstream::source::variant sv = kvp.var;

        switch (sv.which())
        {
            case 4: { // AREA
                auto rs = boost::get<runstream::source::area>(sv);
                if (!rs.complete)
                    break;
                AreaSource *s = new AreaSource;
                //auto s = std::make_unique<AreaSource>();
                s->srcid = kvp.srcid.toStdString();
                s->xs = rs.xs;
                s->ys = rs.ys;
                s->xinit = rs.xinit;
                s->yinit = rs.yinit;
                s->angle = rs.angle;
                s->setGeometry();
                sources.push_back(s);
                break;
            }
            case 5: { // AREAPOLY
                auto rs = boost::get<runstream::source::areapoly>(sv);
                if (!rs.complete)
                    break;
                AreaPolySource *s = new AreaPolySource;
                //auto s = std::make_unique<AreaPolySource>();
                s->srcid = kvp.srcid.toStdString();
                s->geometry = rs.areavert;
                s->setGeometry();
                sources.push_back(s);
                break;
            }
            case 6: { // AREACIRC
                auto rs = boost::get<runstream::source::areacirc>(sv);
                if (!rs.complete)
                    break;
                AreaCircSource *s = new AreaCircSource;
                //auto s = std::make_unique<AreaCircSource>();
                s->srcid = kvp.srcid.toStdString();
                s->xs = rs.xs;
                s->ys = rs.ys;
                s->radius = rs.radius;
                s->nverts = rs.nverts;
                s->setGeometry();
                sources.push_back(s);
                break;
            }
            default: {
                break;
            }
        }
    }

    return;
}
