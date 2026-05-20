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
 * Upload model data to GPU (VBO)
 */
void uploadModelToGPU(Model& m);

/**
 * Load a model file (.obj preferred, legacy .3d supported)
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
