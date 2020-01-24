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

#ifndef SHAPEFILEATTRIBUTEINFO_H
#define SHAPEFILEATTRIBUTEINFO_H

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QMap>

#include <shapelib/shapefil.h>

enum class ShapefileSelectedIndex
{
    SourceID,
    AppDate,
    AppTime,
    AppRate,
    IncDepth
};

class ShapefileAttributeInfo
{
public:
    struct FieldInfo {
        DBFFieldType type;
        QString name;
    };

    int fieldCount = 0;
    int recordCount = 0;
    QMap<int, FieldInfo> fieldInfoMap;

    ShapefileAttributeInfo(const QString& shpfile)
    {
        QFileInfo fi(shpfile);
        QString dbffile = fi.path() + QDir::separator() + fi.baseName() + ".dbf";

        handle = DBFOpen(dbffile.toStdString().c_str(), "rb");
        if (handle <= 0)
            return;

        fieldCount = DBFGetFieldCount(handle);
        recordCount = DBFGetRecordCount(handle);

        for (int i = 0; i < fieldCount; ++i) {
            char pszFieldName[12];
            FieldInfo info;
            info.type = DBFGetFieldInfo(handle, i, pszFieldName, NULL, NULL);
            info.name = QString(pszFieldName);
            fieldInfoMap[i] = info;
        }
    }

    double getNumericField(int index, int record)
    {
        bool validIndex = (index >= 0 && index < fieldCount);
        bool validRecord = (record >= 0 && record < recordCount);
        double res = 0;

        if (handle <= 0 || !validIndex || !validRecord)
            return res;

        if (fieldInfoMap.contains(index)) {
            FieldInfo info = fieldInfoMap.value(index);
            if (info.type == FTInteger) {
                res = DBFReadIntegerAttribute(handle, record, index);
            }
            else if (info.type == FTDouble) {
                res = DBFReadDoubleAttribute(handle, record, index);
            }
        }

        return res;
    }

    QString getStringField(int index, int record)
    {
        bool validIndex = (index >= 0 && index < fieldCount);
        bool validRecord = (record >= 0 && record < recordCount);
        QString res;

        if (handle <= 0 || !validIndex || !validRecord)
            return res;

        if (fieldInfoMap.contains(index)) {
            FieldInfo info = fieldInfoMap.value(index);
            if (info.type == FTString) {
                const char* cstr = DBFReadStringAttribute(handle, record, index);
                res = QString::fromLocal8Bit(cstr);
            }
        }

        return res;
    }

    ~ShapefileAttributeInfo() {
        DBFClose(handle);
    }

private:
    DBFHandle handle;
};

#endif // SHAPEFILEATTRIBUTEINFO_H
