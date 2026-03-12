#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

map<string, list<Vertex>> modelCache;

/**
 * Load a .3d model file
 */
list<Vertex> loadModelFile(const char* filename) {
    list<Vertex> vertices;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open model file " << filename << endl;
        return vertices;
    }

    string line;
    int expectedCount = -1;
    if (getline(file, line)) {
        istringstream iss(line);
        iss >> expectedCount;
    }

    int loadedCount = 0;
    while (getline(file, line)) {
        istringstream iss(line);
        Vertex v;
        if (iss >> v.x >> v.y >> v.z) {
            vertices.push_back(v);
            loadedCount++;
        }
    }

    file.close();
    return vertices;
}

/**
 * Get model vertices (with caching)
 */
list<Vertex> getModelVertices(const string& filename) {
    if (modelCache.find(filename) == modelCache.end()) {
        string modelPath = "../../figures/";
        modelPath += filename;
        modelCache[filename] = loadModelFile(modelPath.c_str());
    }
    return modelCache[filename];
}

/**
 * Clear the model cache
 */
void clearModelCache() {
    modelCache.clear();
}
