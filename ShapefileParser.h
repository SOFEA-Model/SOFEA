#ifndef SHAPEFILEPARSER_H
#define SHAPEFILEPARSER_H

#include <QString>

#include "SourceGroup.h"

class ShapefileParser
{
public:
    static void parseSources(const QString& filename, SourceGroup *sgPtr);
};

#endif // SHAPEFILEPARSER_H
