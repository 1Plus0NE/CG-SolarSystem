#include "../include/figures.h"
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;

struct ScatterSample { float x, y, z; };

inline float rand01() { return (float)rand() / RAND_MAX; }

bool sampleVolume(const string& shape, const vector<float>& p, ScatterSample& out) {
    if (shape == "sphere") {
        float r = p[0] + rand01() * (p[1] - p[0]);
        float u = rand01(), v = rand01();
        float theta = 2.0f * M_PI * u;
        float phi   = acos(2.0f * v - 1.0f);
        out = { r * sin(phi) * cos(theta),
                r * sin(phi) * sin(theta),
                r * cos(phi) };
    } else if (shape == "torus") {
        float R = p[0];
        float r = p[1] + rand01() * (p[2] - p[1]);
        float u = rand01() * 2.0f * M_PI;
        float v = rand01() * 2.0f * M_PI;
        out = { (R + r * cos(v)) * cos(u),
                (R + r * cos(v)) * sin(u),
                 r * sin(v) };
    } else if (shape == "plane") {
        out = { (rand01() - 0.5f) * p[0],
                0.0f,
                (rand01() - 0.5f) * p[1] };
    } else if (shape == "cylinder") {
        float r = p[0] + rand01() * (p[1] - p[0]);
        float h = p[2] + rand01() * (p[3] - p[2]);
        float angle = rand01() * 2.0f * M_PI;
        out = { r * cos(angle), h, r * sin(angle) };
    } else if (shape == "box") {
        float inner = p[0], outer = p[1];
        int face = rand() % 6;
        float depth = inner + rand01() * (outer - inner);
        float u = (rand01() - 0.5f) * 2.0f * outer;
        float v = (rand01() - 0.5f) * 2.0f * outer;
        switch (face) {
            case 0: out = {  depth, u, v }; break;
            case 1: out = { -depth, u, v }; break;
            case 2: out = { u,  depth, v }; break;
            case 3: out = { u, -depth, v }; break;
            case 4: out = { u, v,  depth }; break;
            case 5: out = { u, v, -depth }; break;
        }
    } else {
        cerr << "scatter: unknown volume shape '" << shape << "'" << endl;
        cerr << "Supported: sphere, torus, plane, cylinder, box" << endl;
        return false;
    }
    return true;
}

string transformVertex(const string& vertexLine,
                       float tx, float ty, float tz,
                       float rx, float ry, float rz,
                       float scale) {
    float x, y, z;
    istringstream ss(vertexLine);
    ss >> x >> y >> z;

    x *= scale; y *= scale; z *= scale;

    float y2 =  y * cos(rx) - z * sin(rx);
    float z2 =  y * sin(rx) + z * cos(rx);
    y = y2; z = z2;

    float x2 =  x * cos(ry) + z * sin(ry);
    float z3 = -x * sin(ry) + z * cos(ry);
    x = x2; z = z3;

    float x3 =  x * cos(rz) - y * sin(rz);
    float y3 =  x * sin(rz) + y * cos(rz);
    x = x3; y = y3;

    x += tx; y += ty; z += tz;

    ostringstream out;
    out << x << " " << y << " " << z;
    return out.str();
}

void generateScatter(const string& shape, const vector<float>& params, int num,
                     const vector<string>& modelVerts, float scaleMin, float scaleMax,
                     list<string>& vertices) {
    srand(42);
    for (int i = 0; i < num; i++) {
        ScatterSample pos;
        if (!sampleVolume(shape, params, pos)) return;

        float scale = scaleMin + rand01() * (scaleMax - scaleMin);
        float rx    = rand01() * 2.0f * M_PI;
        float ry    = rand01() * 2.0f * M_PI;
        float rz    = rand01() * 2.0f * M_PI;

        for (const string& v : modelVerts) {
            vertices.push_back(
                transformVertex(v, pos.x, pos.y, pos.z, rx, ry, rz, scale)
            );
        }
    }
}