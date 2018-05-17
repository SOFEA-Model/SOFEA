#include "Scenario.h"
#include "MetFileParser.h"

#include "fmt/format.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <fstream>

const std::map<int, std::string> Scenario::chemicalMap = {
    {0, "1,3-Dichloropropene"},
    {1, "Chloropicrin"},
    {2, "Chloropicrin + 1,3-Dichloropropene"},
    {3, "Chloropicrin + Methyl Bromide"},
    {4, "Metam Sodium / Metam Potassium / Dazomet"},
    {5, "Methyl Bromide"},
    {6, "Sulfuryl Fluoride"},
    {7, "Other"}
};

Scenario::Scenario()
{
    static unsigned int sequenceNumber = 1;
    id = sequenceNumber;
    sequenceNumber++;

    title = fmt::format("Run{:0=2}", id);
    fumigantId = 7; // Other
    decayCoefficient = 0;

    // Meteorological Data
    anemometerHeight = 10;
    windRotation = 0;

    // Dispersion Model
    aermodFlat = true;
    aermodFastArea = false;
    aermodLowWind = false;
    svMin = 0.2;
    wsMin = sqrt(2 * svMin * svMin);
    franMax = 1.0;
}

void Scenario::resetSurfaceFileInfo()
{
    sfInfo = MetFileParser::parseSurfaceFile(surfaceFile);

    if (sfInfo.tmin.is_not_a_date_time())
        return;

    if (sfInfo.tmax.is_not_a_date_time())
        return;

    // Construct QDateTime from boost::posix_time::ptime
    const boost::gregorian::date gdmin = sfInfo.tmin.date();
    const boost::gregorian::date gdmax = sfInfo.tmax.date();
    const boost::posix_time::time_duration ptmin = sfInfo.tmin.time_of_day();
    const boost::posix_time::time_duration ptmax = sfInfo.tmax.time_of_day();

    minTime = QDateTime(QDate(gdmin.year(), gdmin.month(), gdmin.day()),
                                  QTime(ptmin.hours(), 0, 0), Qt::UTC);
    maxTime = QDateTime(QDate(gdmax.year(), gdmax.month(), gdmax.day()),
                                  QTime(ptmax.hours(), 0, 0), Qt::UTC);

    surfaceId = sfInfo.sfloc;
    upperAirId = sfInfo.ualoc;
}

inline std::string aermodTimeString(const QDateTime t)
{
    int hour = t.time().hour() + 1; // AERMOD uses 01-24
    QString hourPart = QString("%1").arg(hour, 2, 10, QChar('0'));
    QString timeStr = t.toString("yyyyMMdd") + hourPart;
    return timeStr.toStdString();
}

std::string Scenario::writeInput() const
{
    // These indexes are used to write out short source and source group
    // identifiers, because AERMOD only supports one hourly emissions
    // file per run.

    int isrc, igrp;

    fmt::MemoryWriter w;

    std::string krun = "RUN";

    // Create options string from flags.
    std::string options;
    if (aermodLowWind)
        options.append("ALPHA CONC");
    else if (aermodFlat | aermodFastArea)
        options.append("CONC");
    else
        options.append("DFAULT CONC");

    if (aermodFlat)
        options.append(" FLAT");
    if (aermodFastArea)
        options.append(" FASTAREA");

    //-------------------------------------------------------------------------
    // CONTROL Pathway
    //-------------------------------------------------------------------------

    w.write("CO STARTING\n");
    w.write("   MODELOPT {0}\n", options);
    if (aermodLowWind) {
        w.write("   LOW_WIND {:06.4f} {:06.4f} {:06.4f}\n", svMin, wsMin, franMax);
    }
    w.write("   FLAGPOLE 1.5\n");
    w.write("   TITLEONE {}\n", title);
    w.write("   AVERTIME PERIOD 1\n");
    w.write("   POLLUTID FUMIGANT\n");
    w.write("   DCAYCOEF {:G}\n", decayCoefficient);
    w.write("   RUNORNOT {}\n", krun);
    w.write("   ERRORFIL error.out\n");
    w.write("CO FINISHED\n\n");

    //-------------------------------------------------------------------------
    // SOURCE Pathway
    //-------------------------------------------------------------------------

    isrc = 0;
    igrp = 1;
    for (const SourceGroup& sg : sourceGroups) {
        if (sg.sources.size() == 0)
            continue;

        w.write("** Source Group {} (G{:0=3})\n", sg.grpid, igrp);
        w.write("SO STARTING\n");
        int isrc0 = isrc;
        for (const Source& s : sg.sources) {
            // LOCATION, SRCPARAM, AREAVERT, HOUREMIS
            w << s.toString(++isrc);
            // BUFRZONE
            for (const std::pair<double, int>& z : sg.zones) {
                QDateTime zoneStart = s.appStart;
                QDateTime zoneEnd = zoneStart.addSecs(z.second * 60 * 60);
                w.write("   BUFRZONE S{:0=3} {: 8.2f} {} {}\n", isrc, z.first,
                        aermodTimeString(zoneStart), aermodTimeString(zoneEnd));
            }
        }

        w.write("\n   SRCGROUP G{:0=3} S{:0=3}-S{:0=3}\n", igrp, ++isrc0, isrc);
        w.write("SO FINISHED\n\n");
        igrp++;
    }

    //-------------------------------------------------------------------------
    // RECEPTOR Pathway
    //-------------------------------------------------------------------------

    w.write("RE STARTING\n");
    igrp = 1;
    for (const SourceGroup& sg : sourceGroups) {
        // Receptor Grids
        int inet = 1;
        for (const ReceptorGrid& grid : sg.grids) {
            w.write("** Source Group {} (G{:0=3}), Grid C{:0=3}\n",
                    sg.grpid, igrp, inet);
            w << grid.toString(igrp, inet);
            inet++;
        }

        // Receptor Nodes
        w.write("** Source Group {} (G{:0=3})\n", sg.grpid, igrp);
        for (const ReceptorNode& node : sg.nodes) {
            w << node.toString();
        }
        w.write("\n");

        // Receptor Rings
        int iarc = 1;
        for (const ReceptorRing& ring : sg.rings) {
            w.write("** Source Group {} (G{:0=3}), Ring R{:0=3}\n",
                    sg.grpid, igrp, iarc);
            w << ring.toString(igrp, iarc);
            iarc++;
        }
        igrp++;
    }
    w.write("RE FINISHED\n\n");

    //-------------------------------------------------------------------------
    // METEOROLOGY Pathway
    //-------------------------------------------------------------------------

    QString startend;
    if (minTime.isValid() && maxTime.isValid())
        startend = minTime.toString("yy MM dd") + " " + maxTime.toString("yy MM dd");

    w.write("ME STARTING\n");
    w.write("   SURFFILE \"{}\"\n", surfaceFile);
    w.write("   PROFFILE \"{}\"\n", upperAirFile);
    w.write("   SURFDATA {} {}\n", surfaceId, minTime.date().year());
    w.write("   UAIRDATA {} {}\n", upperAirId, maxTime.date().year());
    w.write("   PROFBASE {: 6.1f} METERS\n", anemometerHeight);
    w.write("   STARTEND {}\n", startend.toStdString());
    w.write("   WDROTATE {: 5.1f}\n", windRotation);
    w.write("ME FINISHED\n\n");

    //-------------------------------------------------------------------------
    // OUTPUT Pathway
    //-------------------------------------------------------------------------

    w.write("OU STARTING\n");
    w.write("   RECTABLE ALLAVE FIRST\n");

    igrp = 1;
    for (const SourceGroup& sg : sourceGroups) {
        if (sg.sources.size() == 0)
            continue;

        w.write("   POSTFILE  1 G{:0=3} NETCDF {}.nc\n", igrp, sg.grpid);
        igrp++;
    }
    w.write("   SUMMFILE aermod.sum\n");
    w.write("OU FINISHED\n");

    return w.str();
}

void Scenario::writeInputFile(const std::string& path) const
{
    std::ofstream ofs(path);
    ofs << writeInput();
    ofs.close();
}

void Scenario::writeFluxFile(const std::string& path) const
{
    if (!minTime.isValid() || !maxTime.isValid())
        return;

    fmt::MemoryWriter w;
    std::map<QDateTime, std::string> grid;

    // Construct the overall time grid.
    QDateTime t = minTime;
    while (t <= maxTime) {
        int hour = t.time().hour() + 1; // AERMOD uses 01-24
        QString hstr = QString(" %1").arg(hour, 2, 10, QChar('0'));
        QString tstr = t.toString("yy MM dd") + hstr;
        grid[t] = tstr.toStdString();
        t = t.addSecs(60 * 60);
    }

    // Write the flux profile for all source groups.
    for (const SourceGroup& sg : sourceGroups)
    {
        // Expand the reference flux profile to one point per hour.
        std::vector<double> vflux;
        for (const auto& xy : sg.refFlux) {
            std::fill_n(std::back_inserter(vflux), xy.first, xy.second);
        }

        // Determine the number of hours in the flux profile.
        int n = vflux.size();

        // Initialize an array with application started flag for each source.
        std::vector<bool> started(sg.sources.size(), false);

        // Iterate over the time grid.
        for (const auto& kv : grid)
        {
            // Write the flux for each source, applying scaling.
            int isrc = 0;
            for (const Source& s : sg.sources)
            {
                int i = 0; // position in flux vector
                double sf = sg.fluxScaling.fluxScaleFactor(s.appRate, s.appStart, s.incorpDepth);

                double flux;
                if (kv.first == s.appStart) {
                    started[isrc] = true; // application started
                }
                if (started[isrc] && i < n) {
                    flux = vflux[i] * sf;
                    i++;
                }
                else {
                    flux = 0;
                }

                // Write the SO HOUREMIS record.
                w.write("SO HOUREMIS {} S{:0=3} {: 8.6E}\n", kv.second, isrc + 1, flux);

                isrc++;
            }
        }
    }

    // Write the flux file.
    std::ofstream ofs(path);
    ofs << w.str();
    ofs.close();

    return;
}
