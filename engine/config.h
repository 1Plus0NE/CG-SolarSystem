#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <tinyxml2.h>
#include "geometry.h"

using namespace std;
using namespace tinyxml2;

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

extern Group rootGroup;
extern string currentConfigFile;

/**
 * Parse a hex color string (#RRGGBB) into RGB floats [0,1]
 */
void parseHexColor(const char* hex, float& r, float& g, float& b);

/**
 * Parse a group element from XML recursively
 */
Group parseGroup(XMLElement* groupElem);

/**
 * Load XML configuration file
 */
void loadConfigs(const char* filename);

/**
 * Reload the current configuration
 */
void reloadConfig();

#endif // CONFIG_H
