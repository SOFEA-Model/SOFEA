#include "Scenario.h"
#include "MetFileParser.h"
#include "Common.h"
#include "InputFormat.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <fstream>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <QDebug>

#include <fmt/format.h>

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

    // Meteorological Data Defaults
    anemometerHeight = 10;
    windRotation = 0;

    // AERMOD Defaults
    aermodFlat = false;
    aermodFastArea = false;

    // AERMOD Non-DFAULT
    aermodDryDeposition = false;
    aermodDryDplt = true;
    aermodAreaDplt = false;
    aermodWetDeposition = false;
    aermodWetDplt = true;
    aermodGDSeasons = {4, 4, 5, 5, 5, 1, 1, 1, 2, 2, 2, 4};
    std::fill_n(aermodGDLandUse.begin(), 36, 2);

    // Gas Deposition
    aermodGDReact = 0;
    aermodGDSeasonF = {1.0, 0.5, 1.0, 1.0, 0.25};
    aermodGDVelocityEnabled = false;
    aermodGDVelocity = 0.01;

    // AERMOD Alpha
    aermodLowWind = false;
    aermodSVmin = 0.2;
    aermodWSmin = sqrt(2 * aermodSVmin * aermodSVmin);
    aermodFRANmax = 1.0;

    // Output
    averagingPeriods = {1, 24};

    // Projection Defaults
    hUnitsCode = "9001"; // meters
    vUnitsCode = "9001"; // meters
}

Scenario::Scenario(const Scenario& rhs)
{
    this->id = rhs.id;
    this->title = rhs.title;
    this->fumigantId = rhs.fumigantId;
    this->decayCoefficient = rhs.decayCoefficient;
    this->upperAirFile = rhs.upperAirFile;
    this->surfaceFile = rhs.upperAirFile;
    this->anemometerHeight = rhs.anemometerHeight;
    this->windRotation = rhs.windRotation;
    this->sfInfo = rhs.sfInfo;
    this->minTime = rhs.minTime;
    this->maxTime = rhs.minTime;
    this->surfaceId = rhs.surfaceId;
    this->upperAirId = rhs.upperAirId;
    this->aermodFlat = rhs.aermodFlat;
    this->aermodFastArea = rhs.aermodFastArea;
    this->aermodDryDeposition = rhs.aermodDryDeposition;
    this->aermodDryDplt = rhs.aermodDryDplt;
    this->aermodAreaDplt = rhs.aermodAreaDplt;
    this->aermodWetDeposition = rhs.aermodWetDeposition;
    this->aermodWetDplt = rhs.aermodWetDplt;
    this->aermodGDSeasons = rhs.aermodGDSeasons;
    this->aermodGDLandUse = rhs.aermodGDLandUse;
    this->aermodGDReact = rhs.aermodGDReact;
    this->aermodGDSeasonF = rhs.aermodGDSeasonF;
    this->aermodGDVelocityEnabled = rhs.aermodGDVelocityEnabled;
    this->aermodGDVelocity = rhs.aermodGDVelocity;
    this->aermodLowWind = rhs.aermodLowWind;
    this->aermodSVmin = rhs.aermodSVmin;
    this->aermodWSmin = rhs.aermodWSmin;
    this->aermodFRANmax = rhs.aermodFRANmax;
    this->averagingPeriods = rhs.averagingPeriods;
    this->receptors = rhs.receptors;

    // Deep copy flux profiles
    using FluxProfileWeakPtr = std::weak_ptr<FluxProfile>;
    std::map<FluxProfilePtr, FluxProfileWeakPtr> fpwptrMap;
    this->fluxProfiles.reserve(rhs.fluxProfiles.size());
    for (const auto& fp : rhs.fluxProfiles) {
        if (fp != nullptr) {
            auto clone = std::make_shared<FluxProfile>(*fp);
            this->fluxProfiles.emplace_back(clone);
            fpwptrMap[fp] = FluxProfileWeakPtr(clone);
        }
    }

    // Deep copy source groups
    using SourceGroupWeakPtr = std::weak_ptr<SourceGroup>;
    std::map<SourceGroupPtr, SourceGroupWeakPtr> sgwptrMap;
    this->sourceGroups.reserve(rhs.sourceGroups.size());
    for (const auto& sg : rhs.sourceGroups) {
        if (sg != nullptr) {
            auto clone = std::make_shared<SourceGroup>(*sg);
            this->sourceGroups.emplace_back(clone);

            // Update flux profile weak pointers
            decltype(clone->fluxProfile.data) newfpdata;
            for (auto&& item : clone->fluxProfile.data) {
                auto fp = item.first.lock();
                if (fpwptrMap.count(fp)) {
                    newfpdata[fpwptrMap.at(fp)] = item.second;
                }
            }

            clone->fluxProfile.data = newfpdata;

            for (auto& s : clone->sources) {
                auto fp = s.fluxProfile.lock();
                s.fluxProfile = fpwptrMap.count(fp) ? fpwptrMap.at(fp) : FluxProfileWeakPtr();
            }

            sgwptrMap[sg] = SourceGroupWeakPtr(clone);
        }
    }

    // Update source group weak pointers
    for (auto&& rg : this->receptors) {
        if (rg.type() == typeid(ReceptorRingGroup)) {
            auto& ring = boost::get<ReceptorRingGroup>(rg);
            auto sg = ring.sgPtr.lock();
            if (sgwptrMap.count(sg)) {
                ring.sgPtr = sgwptrMap.at(sg);
            }
        }
    }

    this->domain = rhs.domain;
    this->conversionCode = rhs.conversionCode;
    this->hUnitsCode = rhs.hUnitsCode;
    this->hDatumCode = rhs.hDatumCode;
    this->vUnitsCode = rhs.vUnitsCode;
    this->vDatumCode = rhs.vDatumCode;
}

void Scenario::resetSurfaceFileInfo()
{
    MetFileParser parser(surfaceFile);
    sfInfo = parser.getSurfaceInfo();

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

inline std::string aermodTimeString(const QDateTime& t)
{
    int hour = t.time().hour() + 1; // AERMOD uses 01-24
    QString hourPart = QString("%1").arg(hour, 2, 10, QChar('0'));
    QString timeStr = t.toString("yyyyMMdd") + hourPart;
    return timeStr.toStdString();
}

std::string Scenario::writeInput() const
{
    std::string krun = "RUN"; // RUNORNOT

    fmt::memory_buffer w;

    fmt::format_to(w, "** \n");
    fmt::format_to(w, "** Generated by SOFEA " SOFEA_VERSION_STRING "\n");
    fmt::format_to(w, "** \n");

    //-------------------------------------------------------------------------
    // Model Options
    //-------------------------------------------------------------------------

    // Create options string from flags.
    std::string options;

    // Determine ALPHA, NonDFAULT, DFAULT
    if (aermodLowWind)
        options.append("ALPHA CONC");
    else if (aermodFlat | aermodFastArea | aermodDryDeposition | aermodWetDeposition)
        options.append("CONC");
    else
        options.append("DFAULT CONC");

    // Deposition Options
    if (aermodDryDeposition & aermodWetDeposition)
        options.append(" DEPOS");
    if (aermodDryDeposition)
        options.append(" DDEP");
    if (aermodWetDeposition)
        options.append(" WDEP");

    // General Options
    if (aermodFlat)
        options.append(" FLAT");
    if (aermodFastArea)
        options.append(" FASTAREA");

    // Depletion Options
    if (aermodDryDeposition)
        aermodDryDplt ? options.append(" DRYDPLT")
                      : options.append(" NODRYDPLT");
    if (aermodWetDeposition)
        aermodWetDplt ? options.append(" WETDPLT")
                      : options.append(" NOWETDPLT");
    if (aermodDryDeposition & aermodDryDplt & aermodAreaDplt)
        options.append(" AREADPLT");

    //-------------------------------------------------------------------------
    // CONTROL Pathway
    //-------------------------------------------------------------------------

    fmt::format_to(w, "CO STARTING\n");
    fmt::format_to(w, "   MODELOPT {0}\n", options);
    if (aermodLowWind) {
        fmt::format_to(w, "   LOW_WIND {:06.4f} {:06.4f} {:06.4f}\n", aermodSVmin, aermodWSmin, aermodFRANmax);
    }
    fmt::format_to(w, "   TITLEONE {}\n", title);
    if (averagingPeriods.size() > 0) {
        fmt::format_to(w, "   AVERTIME");
        for (int i : averagingPeriods)
            fmt::format_to(w, " {}", i);
        fmt::format_to(w, "\n");
    }
    fmt::format_to(w, "   POLLUTID FUMIGANT\n");
    fmt::format_to(w, "   DCAYCOEF {:G}\n", decayCoefficient);
    if (aermodDryDeposition & aermodGDVelocityEnabled & !aermodWetDeposition) {
        fmt::format_to(w, "   GASDEPVD {:<1.4f}\n", aermodGDVelocity);
    }
    if (aermodDryDeposition & !aermodGDVelocityEnabled) {
        fmt::format_to(w, "   GDSEASON");
        for (int i : aermodGDSeasons) {
            fmt::format_to(w, " {}", i);
        }
        fmt::format_to(w, "\n");
        fmt::format_to(w, "   GDLANUSE");
        for (int i : aermodGDLandUse) {
            fmt::format_to(w, " {}", i);
        }
        fmt::format_to(w, "\n");
    }
    fmt::format_to(w, "   RUNORNOT {}\n", krun);
    fmt::format_to(w, "   ERRORFIL error.out\n");
    fmt::format_to(w, "CO FINISHED\n\n");

    //-------------------------------------------------------------------------
    // SOURCE Pathway
    //-------------------------------------------------------------------------

    // These indexes are used to write out short source and source group
    // identifiers, because AERMOD only supports one hourly emissions
    // file per run.

    int isrc, igrp;

    fmt::format_to(w, "SO STARTING\n");

    // Write parameters for each source.
    // LOCATION, SRCPARAM, AREAVERT, HOUREMIS, GASDEPOS, BUFRZONE
    isrc = 0;
    igrp = 0;
    for (const SourceGroupPtr sgptr : sourceGroups) {
        if (sgptr->sources.size() == 0)
            continue;

        fmt::format_to(w, "** Source Group {} (G{:0=3})\n", sgptr->grpid, ++igrp);
        for (const Source& s : sgptr->sources)
        {
            // LOCATION, SRCPARAM, AREAVERT, HOUREMIS
            fmt::format_to(w, s.toString(++isrc));

            // GASDEPOS
            if ((aermodDryDeposition | aermodWetDeposition) &
                !aermodGDVelocityEnabled) {
                fmt::format_to(w, "   GASDEPOS S{:0=3} {} {} {} {}\n", isrc,
                        s.airDiffusion, s.waterDiffusion,
                        s.cuticularResistance, s.henryConstant);
            }

            // BUFRZONE
            if (sgptr->enableBufferZones) {
                BufferZone zref;
                zref.appRateThreshold = s.appRate;
                zref.areaThreshold = s.area();

                auto it = std::find_if(sgptr->zones.begin(), sgptr->zones.end(), [&zref](const BufferZone& z) {
                   return (z.areaThreshold >= zref.areaThreshold) && (z.appRateThreshold >= zref.appRateThreshold);
                });

                if (it != sgptr->zones.end()) {
                    int totalHours = it->duration;
                    QDateTime zoneStart = s.appStart;
                    QDateTime zoneEnd = zoneStart.addSecs(totalHours * 60 * 60);
                    fmt::format_to(w, "   BUFRZONE S{:0=3} {: 8.2f} {} {}\n", isrc, it->distance,
                            aermodTimeString(zoneStart), aermodTimeString(zoneEnd));
                }
            }
        }
        fmt::format_to(w, "\n");
    }

    // Write SRCGROUP for each source range.
    fmt::format_to(w, "   SRCGROUP ALL\n");
    isrc = 0;
    igrp = 0;
    for (const SourceGroupPtr sgptr : sourceGroups) {
        int ns = sgptr->sources.size();
        if (ns == 0)
            continue;

        int isrc0 = ++isrc;
        isrc += ns - 1;
        fmt::format_to(w, "   SRCGROUP G{:0=3} S{:0=3}-S{:0=3}\n", ++igrp, isrc0, isrc);
    }

    fmt::format_to(w, "SO FINISHED\n\n");

    //-------------------------------------------------------------------------
    // RECEPTOR Pathway
    //-------------------------------------------------------------------------

    fmt::format_to(w, "RE STARTING\n");
    for (const auto& group : receptors)
        fmt::format_to(w, "{}", group);
    fmt::format_to(w, "RE FINISHED\n\n");

    //-------------------------------------------------------------------------
    // METEOROLOGY Pathway
    //-------------------------------------------------------------------------

    QString startend;
    if (minTime.isValid() && maxTime.isValid())
        startend = minTime.toString("yy MM dd") + " " + maxTime.toString("yy MM dd");

    fmt::format_to(w, "ME STARTING\n");
    fmt::format_to(w, "   SURFFILE \"{}\"\n", MetFileParser::absolutePath(surfaceFile));
    fmt::format_to(w, "   PROFFILE \"{}\"\n", MetFileParser::absolutePath(upperAirFile));
    fmt::format_to(w, "   SURFDATA {} {}\n", surfaceId, minTime.date().year());
    fmt::format_to(w, "   UAIRDATA {} {}\n", upperAirId, maxTime.date().year());
    fmt::format_to(w, "   PROFBASE {: 6.1f} METERS\n", anemometerHeight);
    fmt::format_to(w, "   STARTEND {}\n", startend.toStdString());
    fmt::format_to(w, "   WDROTATE {: 5.1f}\n", windRotation);
    fmt::format_to(w, "ME FINISHED\n\n");

    //-------------------------------------------------------------------------
    // OUTPUT Pathway
    //-------------------------------------------------------------------------

    fmt::format_to(w, "OU STARTING\n");
    fmt::format_to(w, "   RECTABLE ALLAVE FIRST\n");
    fmt::format_to(w, "   SUMMFILE summary.txt\n");
    igrp = 1;
    for (const SourceGroupPtr sgptr : sourceGroups) {
        if (sgptr->sources.size() == 0)
            continue;

        for (int i : averagingPeriods)
            fmt::format_to(w, "   POSTFILE {: =2} G{:0=3} NETCDF postfile.nc\n", i, igrp);

        igrp++;
    }

    for (int period : averagingPeriods)
        fmt::format_to(w, "   POSTFILE {: =2} ALL  NETCDF postfile.nc\n", period);

    fmt::format_to(w, "OU FINISHED\n");

    return fmt::to_string(w);
}

void Scenario::writeInputFile(const std::string& path) const
{
    std::ofstream ofs(path);
    ofs << writeInput();
    ofs.close();
}

void Scenario::writeFluxFile(const std::string& path) const
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Model");

    if (!minTime.isValid() || !maxTime.isValid()) {
        BOOST_LOG_TRIVIAL(error) << "Invalid time range";
        return; // FIXME: throw an exception
    }

    // Make sure that each source has a valid flux profile.
    bool missingProfile = false;
    for (const SourceGroupPtr sgptr : sourceGroups) {
        for (const Source &s : sgptr->sources) {
            if (s.fluxProfile.expired()) {
                BOOST_LOG_TRIVIAL(error) << "Missing flux profile for source " << s.srcid;
                missingProfile = true;
            }
        }
    }

    if (missingProfile)
        return; // FIXME: throw an exception

    fmt::memory_buffer w;
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

    // Generate and cache all expanded reference flux profiles.
    using FluxProfileWeakPtr = std::weak_ptr<FluxProfile>;
    using FluxMapCompare = std::owner_less<FluxProfileWeakPtr>;
    using FluxMap = std::map<FluxProfileWeakPtr, std::vector<double>, FluxMapCompare>;
    FluxMap exRefFluxMap;

    for (const auto& fp : fluxProfiles)
    {
        // Expand the reference flux profile to one point per hour.
        std::vector<double> exRefFlux;
        for (const auto& xy : fp->refFlux) {
            std::fill_n(std::back_inserter(exRefFlux), xy.first, xy.second);
        }

        FluxProfileWeakPtr fpw = fp;
        exRefFluxMap[fpw] = exRefFlux;
    }

    // Determine the overall number of sources across source groups.
    int nsrc = 0;
    for (const SourceGroupPtr sgptr : sourceGroups)
        nsrc += sgptr->sources.size();

    // Initialize a vector with application started flag for each source.
    std::vector<bool> started(nsrc, false);

    // Initialize a vector with position in reference flux vector for each source.
    std::vector<int> index(nsrc, 0);

    // Write the flux profile for all sources. Must be in order of hour, then source.
    for (const auto& kv : grid)
    {
        // Initialize the source counter.
        int isrc = 0;

        for (const SourceGroupPtr sgptr : sourceGroups)
        {
            for (const Source &s : sgptr->sources)
            {
                const auto fp = s.fluxProfile.lock();
                if (!fp) {
                    BOOST_LOG_TRIVIAL(error) << "Failed to acquire read lock";
                    return; // FIXME: throw an exception
                }

                // Calculate the overall flux scale factor.
                double sf = fp->fluxScaleFactor(s.appRate, s.appStart, s.incorpDepth);

                // Get the cached reference flux profile for this source.
                if (!exRefFluxMap.count(s.fluxProfile)) {
                    BOOST_LOG_TRIVIAL(error) << "Failed to locate source flux profile";
                    return; // FIXME: throw an exception
                }
                std::vector<double> exRefFlux = exRefFluxMap.at(s.fluxProfile);

                // Determine the number of hours in the expanded flux profile.
                int n = exRefFlux.size();

                // Calculate flux.
                double flux;
                if (kv.first == s.appStart) {
                    started[isrc] = true;
                }
                if (started[isrc] && index[isrc] < n) {
                    flux = exRefFlux[index[isrc]] * sf;
                    index[isrc]++;
                }
                else {
                    flux = 0;
                }

                // Write the SO HOUREMIS record and increment source counter.
                fmt::format_to(w, "SO HOUREMIS {} S{:0=3} {: 8.6E}\n", kv.second, ++isrc, flux);
            }
        }
    }

    // Write the flux file.
    std::ofstream ofs(path);
    ofs << fmt::to_string(w);
    ofs.close();

    return;
}

