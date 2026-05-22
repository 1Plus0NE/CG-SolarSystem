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

bool uploadModelToGPU(Model& m);  // returns false on empty/allocation/upload failure
void freeModelGPU(Model& m);
list<Vertex> loadModelFile(const char* filename);
list<Vertex> getModelVertices(const string& filename);
void clearModelCache();

#endif // MODEL_H
