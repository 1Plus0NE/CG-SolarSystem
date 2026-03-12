#ifndef MODEL_H
#define MODEL_H

#include <list>
#include <string>
#include <map>
#include "geometry.h"

using namespace std;

// ============================================================================
// MODEL MANAGEMENT
// ============================================================================

extern map<string, list<Vertex>> modelCache;

/**
 * Load a .3d model file
 */
list<Vertex> loadModelFile(const char* filename);

/**
 * Get model vertices (with caching)
 */
list<Vertex> getModelVertices(const string& filename);

/**
 * Clear the model cache
 */
void clearModelCache();

#endif // MODEL_H
