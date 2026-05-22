#pragma once
#include <list>
#include <string>
#include <vector>

using namespace std;

struct ScatterSample { float x, y, z; };
inline float rand01() { return (float)rand() / RAND_MAX; }

struct Vert {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};
using VertList = vector<Vert>;

bool verifyMetric(const string& name, float value, float min);
bool sampleVolume(const string& shape, const vector<float>& params, ScatterSample& out);
string transformVertex(const string& vertexLine,
                       float tx, float ty, float tz,
                       float rx, float ry, float rz,
                       float scale);
