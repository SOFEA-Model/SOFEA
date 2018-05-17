#ifndef RUNSTREAMPARSER_H
#define RUNSTREAMPARSER_H

#include <list>

#include <QString>
#include <QPolygonF>

#include "SourceGroup.h"

class RunstreamParser
{
public:
    static void parseSources(const QString& inputFile, boost::ptr_vector<Source>& sources);
};

#endif // RUNSTREAMPARSER_H
