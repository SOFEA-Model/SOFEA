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
#include <boost/log/attributes/scoped_attribute.hpp>

// FIXME: allow reading without SO STARTING / SO FINISHED
// TODO: add DISCCART and EVALCART receptor parsing

bool parseInternalLocation(QTextStream& is, runstream::source::container& sc)
{
    QString srcid;
    QString st;

    is >> srcid >> st;

    if (is.status() != QTextStream::Ok)
        return false; // read failure

    if (st == "AREA") {
        runstream::source::area s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return false; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }
    else if (st == "AREAPOLY") {
        runstream::source::areapoly s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return false; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }
    else if (st == "AREACIRC") {
        runstream::source::areacirc s;
        is >> s.xs >> s.ys;
        if (is.status() != QTextStream::Ok)
            return false; // read failure
        is >> s.zs;
        runstream::source::kvp kvp(srcid, s);
        sc.push_back(kvp);
    }

    return true;
}

bool parseInternalSrcParam(QTextStream& is, runstream::source::container& sc)
{
    typedef runstream::source::container::nth_index<1>::type indexed_view;

    QString srcid;
    is >> srcid;
    if (is.status() != QTextStream::Ok)
        return false; // read failure

    indexed_view& v = sc.get<1>();
    indexed_view::iterator it = v.find(srcid);
    if (it == v.end())
        return false; // id not found

    runstream::source::kvp kvp = *it;
    runstream::source::variant sv = kvp.var;

    switch (sv.which())
    {
    case 4: { // AREA
        auto s = boost::get<runstream::source::area>(sv);
        is >> s.aremis >> s.relhgt >> s.xinit;
        if (is.status() != QTextStream::Ok)
            return false; // read failure
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
            return false; // read failure
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
            return false; // read failure
        if (!is.atEnd())
            is >> s.nverts >> s.szinit;
        s.complete = true; // terminal parse
        kvp.var = s;
        v.replace(it, kvp); // update record
        break;
    }
    default:
        break;
    }

    return true;
}

bool parseInternalAreaVert(QTextStream& is, runstream::source::container& sc)
{
    typedef runstream::source::container::nth_index<1>::type indexed_view;

    QString srcid;
    is >> srcid;
    if (is.status() != QTextStream::Ok)
        return false; // read failure

    indexed_view& v = sc.get<1>();
    indexed_view::iterator it = v.find(srcid);
    if (it == v.end())
        return false; // id not found

    runstream::source::kvp kvp = *it;
    runstream::source::variant sv = kvp.var;

    if (sv.which() != 5)
        return false; // not areapoly type

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

    return true;
}

void RunstreamParser::parseSources(const QString& filename, SourceGroup *sgPtr)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Import");

    runstream::source::container sc;

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        BOOST_LOG_TRIVIAL(error) << "Error opening runstream file";
        return;
    }

    // Begin File I/O
    bool block = false;
    int iline = 0;
    while (!file.atEnd())
    {
        QString line = file.readLine();
        iline++;
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
                bool ok = parseInternalLocation(is, sc);
                if (!ok)
                    BOOST_LOG_TRIVIAL(error) << "LOCATION parse failure at line " << iline;
            }
            else if (keyword == "SRCPARAM") {
                bool ok = parseInternalSrcParam(is, sc);
                if (!ok)
                    BOOST_LOG_TRIVIAL(error) << "SRCPARAM parse failure at line " << iline;

            }
            else if (keyword == "AREAVERT") {
                bool ok = parseInternalAreaVert(is, sc);
                if (!ok)
                    BOOST_LOG_TRIVIAL(error) << "LOCATION parse failure at line " << iline;
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
        case 0: {  // POINT
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (POINT)";
            break;
        }
        case 1: {  // POINTCAP
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (POINTCAP)";
            break;
        }
        case 2: {  // POINTHOR
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (POINTHOR)";
            break;
        }
        case 3: {  // VOLUME
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (VOLUME)";
            break;
        }
        case 4: { // AREA
            auto rs = boost::get<runstream::source::area>(sv);
            if (!rs.complete)
                break;
            AreaSource *s = new AreaSource;
            s->srcid = kvp.srcid.toStdString();
            s->xs = rs.xs;
            s->ys = rs.ys;
            s->xinit = rs.xinit;
            s->yinit = rs.yinit;
            s->angle = rs.angle;
            s->setGeometry();
            sgPtr->initSource(s);
            sgPtr->sources.push_back(s);
            break;
        }
        case 5: { // AREAPOLY
            auto rs = boost::get<runstream::source::areapoly>(sv);
            if (!rs.complete)
                break;
            AreaPolySource *s = new AreaPolySource;
            s->srcid = kvp.srcid.toStdString();
            s->geometry = rs.areavert;
            s->setGeometry();
            sgPtr->initSource(s);
            sgPtr->sources.push_back(s);
            break;
        }
        case 6: { // AREACIRC
            auto rs = boost::get<runstream::source::areacirc>(sv);
            if (!rs.complete)
                break;
            AreaCircSource *s = new AreaCircSource;
            s->srcid = kvp.srcid.toStdString();
            s->xs = rs.xs;
            s->ys = rs.ys;
            s->radius = rs.radius;
            s->nverts = rs.nverts;
            s->setGeometry();
            sgPtr->initSource(s);
            sgPtr->sources.push_back(s);
            break;
        }
        case 7: {  // OPENPIT
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (OPENPIT)";
            break;
        }
        case 8: {  // LINE
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (LINE)";
            break;
        }
        case 9: {  // BUOYLINE
            BOOST_LOG_TRIVIAL(warning) << "Unsupported source type (BUOYLINE)";
            break;
        }
        default:
            break;
        }
    }

    return;
}
