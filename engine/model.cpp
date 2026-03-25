#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>

using namespace std;

map<string, list<Vertex>> modelCache;

/**
 * Upload model data to GPU (VBO)
 */
void uploadModelToGPU(Model& m) {
    // Converte list<Vertex> para array contíguo
    vector<float> buf;
    buf.reserve(m.vertices.size() * 3);
    for (const auto& v : m.vertices) {
        buf.push_back(v.x);
        buf.push_back(v.y);
        buf.push_back(v.z);
    }
    m.vertexCount = (int)m.vertices.size();

    GLenum usage = (m.renderMode == STATIC)
                 ? GL_STATIC_DRAW
                 : GL_DYNAMIC_DRAW;

    if (!m.gpuReady) {
        glGenVertexArrays(1, &m.vaoId);
        glGenBuffers(1, &m.vboId);
        m.gpuReady = true;
    }

    glBindVertexArray(m.vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m.vboId);
    glBufferData(GL_ARRAY_BUFFER,
                 buf.size() * sizeof(float),
                 buf.data(), usage);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    m.isDirty = false;
}

void freeModelGPU(Model& m) {
    if (!m.gpuReady) return;
    glDeleteVertexArrays(1, &m.vaoId);
    glDeleteBuffers(1, &m.vboId);
    m.gpuReady = false;
}

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
