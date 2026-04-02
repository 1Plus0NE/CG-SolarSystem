#pragma once
#include <list>
#include <string>
#include <vector>

using namespace std;

struct ScatterSample { float x, y, z; };
inline float rand01() { return (float)rand() / RAND_MAX; }

void addVertex(list<string>& vertices, float x, float y, float z);
void generateTriangle(list<string>& vertices,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float x3, float y3, float z3);
void generateQuad(list<string>& vertices,
                  float x1, float y1, float z1,
                  float x2, float y2, float z2,
                  float x3, float y3, float z3,
                  float x4, float y4, float z4);
bool verifyMetric(const string& name, float value, float min);
bool sampleVolume(const string& shape, const vector<float>& params, ScatterSample& out);
string transformVertex(const string& vertexLine,
                       float tx, float ty, float tz,
                       float rx, float ry, float rz,
                       float scale);