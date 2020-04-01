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

#include "MeteorologyStationData.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QtSql>

#include <QDebug>

#include <utility>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

enum class StationIDType
{
    Unknown,
    USAF,
    WBAN
};

std::pair<StationIDType, QString> parseStationId(const QString& id)
{
    // Check for USAF "A12345" format.
    QRegularExpression re("^\\s*A\\d\\d\\d\\d\\d$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(id);
    if (match.hasMatch())
        return std::make_pair(StationIDType::USAF, id.trimmed());

    // Heuristic check for USAF or WBAN ID in known range.
    int value = id.toInt();
    if (value >= 100 && value <= 96409)
        return std::make_pair(StationIDType::WBAN, QString::number(value).rightJustified(5, '0'));
    else if (value >= 621010 && value <= 998499)
        return std::make_pair(StationIDType::USAF, QString::number(value).rightJustified(6, '0'));
    else
        return std::make_pair(StationIDType::Unknown, QString());
}

QSqlDatabase& MeteorologyStationDatabase::instance()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General")

    static QString dbPath = QDir::cleanPath(QCoreApplication::applicationDirPath()
        + QDir::separator() + "share/sofea/stations.db");

    static QSqlDatabase db;

    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", "stations.db");
        db.setDatabaseName(dbPath);
        db.setConnectOptions("QSQLITE_OPEN_READONLY");
        if (!db.open())
            BOOST_LOG_TRIVIAL(error) << db.lastError().text().toLocal8Bit().constData();
    }

    return db;
}

StationQueryModel::StationQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
{
    setHeaderData(0,  Qt::Horizontal, QObject::tr("USAF ID"));
    setHeaderData(1,  Qt::Horizontal, QObject::tr("WBAN ID"));
    setHeaderData(2,  Qt::Horizontal, QObject::tr("Begin Date"));
    setHeaderData(3,  Qt::Horizontal, QObject::tr("End Date"));
    setHeaderData(4,  Qt::Horizontal, QObject::tr("Station Name"));
    setHeaderData(5,  Qt::Horizontal, QObject::tr("State"));
    setHeaderData(6,  Qt::Horizontal, QObject::tr("County"));
    setHeaderData(7,  Qt::Horizontal, QObject::tr("UTC Offset"));
    setHeaderData(8,  Qt::Horizontal, QObject::tr("Observing Programs"));
    setHeaderData(9,  Qt::Horizontal, QObject::tr("Platform"));
    setHeaderData(10, Qt::Horizontal, QObject::tr("Latitude"));
    setHeaderData(11, Qt::Horizontal, QObject::tr("Longitude"));
    setHeaderData(12, Qt::Horizontal, QObject::tr("Ground Elevation"));
    setHeaderData(13, Qt::Horizontal, QObject::tr("Ground Elevation Unit"));
    setHeaderData(14, Qt::Horizontal, QObject::tr("Horizontal Datum"));
    setHeaderData(15, Qt::Horizontal, QObject::tr("Vertical Datum"));
    setHeaderData(16, Qt::Horizontal, QObject::tr("Lat/Lon Source"));
}

AnemometerQueryModel::AnemometerQueryModel(QObject *parent)
    : QSqlQueryModel(parent)
{
    setHeaderData(0,  Qt::Horizontal, QObject::tr("USAF ID"));
    setHeaderData(1,  Qt::Horizontal, QObject::tr("WBAN ID"));
    setHeaderData(2,  Qt::Horizontal, QObject::tr("Begin Date"));
    setHeaderData(3,  Qt::Horizontal, QObject::tr("End Date"));
    setHeaderData(4,  Qt::Horizontal, QObject::tr("Anemometer Height"));
    setHeaderData(5,  Qt::Horizontal, QObject::tr("Anemometer Height Unit"));
}

QSqlQuery MeteorologyStationData::anemometerQuery(const QString& id, const QDateTime& begin, const QDateTime& end)
{
    QSqlDatabase& db = MeteorologyStationDatabase::instance();
    QSqlQuery q(db);
    auto parsed = parseStationId(id);
    switch (parsed.first) {
    case StationIDType::USAF:
        q.prepare("SELECT * FROM anemometer_view WHERE usaf_id = ? AND begin_date <= ? AND end_date >= ?");
        break;
    case StationIDType::WBAN:
        q.prepare("SELECT * FROM anemometer_view WHERE wban_id = ? AND begin_date <= ? AND end_date >= ?");
        break;
    default:
        return q;
    }
    q.addBindValue(parsed.second);
    q.addBindValue(begin.toString("yyyyMMdd"));
    q.addBindValue(end.toString("yyyyMMdd"));
    return q;
}

QSqlQuery MeteorologyStationData::stationQuery(const QString& id, const QDateTime& begin, const QDateTime& end)
{
    QSqlDatabase& db = MeteorologyStationDatabase::instance();
    QSqlQuery q(db);
    auto parsed = parseStationId(id);
    switch (parsed.first) {
    case StationIDType::USAF:
        q.prepare("SELECT * FROM station_view WHERE usaf_id = ? AND begin_date <= ? AND end_date >= ?");
        break;
    case StationIDType::WBAN:
        q.prepare("SELECT * FROM station_view WHERE wban_id = ? AND begin_date <= ? AND end_date >= ?");
        break;
    default:
        return q;
    }
    q.addBindValue(parsed.second);
    q.addBindValue(begin.toString("yyyyMMdd"));
    q.addBindValue(end.toString("yyyyMMdd"));
    return q;
}
