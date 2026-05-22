#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <tinyxml2.h>
#include "application_state.h"
#include "color_parser.h"

using namespace std;
using namespace tinyxml2;

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

extern string currentConfigFile;
extern float  sceneGlobalAmbient[4];

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
