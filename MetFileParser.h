#ifndef METFILEPARSER_H
#define METFILEPARSER_H

#include <string>
#include <vector>

#include "Scenario.h"

class MetFileParser
{
public:
    static SurfaceFileInfo parseSurfaceFile(const std::string& filename);
};

#endif // METFILEPARSER_H
