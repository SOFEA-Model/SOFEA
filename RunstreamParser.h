#ifndef RUNSTREAMPARSER_H
#define RUNSTREAMPARSER_H

#include <list>

#include <QString>
#include <QPolygonF>

#include "SourceGroup.h"

class RunstreamParser
{
public:
    static void parseSources(const QString& filename, SourceGroup *sgPtr);
};

#endif // RUNSTREAMPARSER_H
