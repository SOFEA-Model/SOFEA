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

#include <QDateTime>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>

class StationQueryModel : QSqlQueryModel
{
public:
    enum Column {
        USAFID,
        WBANID,
        BeginDate,
        EndDate,
        StationName,
        State,
        County,
        UTCOffset,
        ObservingPrograms,
        Platform,
        Latitude,
        Longitude,
        GroundElevation,
        GroundElevationUnit,
        HorizontalDatum,
        VerticalDatum,
        LatLonSource
    };

    StationQueryModel(QObject *parent = nullptr);
};

class AnemometerQueryModel : QSqlQueryModel
{
public:
    enum Column {
        USAFID,
        WBANID,
        BeginDate,
        EndDate,
        AnemometerHeight,
        AnemometerHeightUnit
    };

    AnemometerQueryModel(QObject *parent = nullptr);
};

class MeteorologyStationDatabase
{
public:
    static QSqlDatabase& instance();

    MeteorologyStationDatabase(const MeteorologyStationDatabase&) = delete;
    MeteorologyStationDatabase& operator=(const MeteorologyStationDatabase&) = delete;
    MeteorologyStationDatabase(MeteorologyStationDatabase&&) = delete;
    MeteorologyStationDatabase& operator=(MeteorologyStationDatabase&&) = delete;

private:
    MeteorologyStationDatabase() = default;
    ~MeteorologyStationDatabase() = default;
};

class MeteorologyStationData
{
public:
    static QSqlQuery stationQuery(const QString& id, const QDateTime& begin, const QDateTime& end);
    static QSqlQuery anemometerQuery(const QString& id, const QDateTime& begin, const QDateTime& end);
};



