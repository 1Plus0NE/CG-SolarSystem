#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generatePlane(float length, int divisions, VertList& out) {
    float half = length / 2.0f;
    float step = length / divisions;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step,       x1 = -half + (i + 1) * step;
            float z0 = -half + j * step,        z1 = -half + (j + 1) * step;
            float u0 = (float)i / divisions,    u1 = (float)(i + 1) / divisions;
            float vv0 = (float)j / divisions,   vv1 = (float)(j + 1) / divisions;

            // Top face (normal +Y), CCW from above
            Vert p1t = {x0, 0, z0, u0, vv0,  0, 1, 0};
            Vert p2t = {x1, 0, z0, u1, vv0,  0, 1, 0};
            Vert p3t = {x0, 0, z1, u0, vv1,  0, 1, 0};
            Vert p4t = {x1, 0, z1, u1, vv1,  0, 1, 0};
            out.push_back(p1t); out.push_back(p4t); out.push_back(p2t);
            out.push_back(p1t); out.push_back(p3t); out.push_back(p4t);

            // Bottom face (normal -Y), CCW from below
            Vert p1b = {x1, 0, z0, u1, vv0,  0, -1, 0};
            Vert p2b = {x0, 0, z0, u0, vv0,  0, -1, 0};
            Vert p3b = {x1, 0, z1, u1, vv1,  0, -1, 0};
            Vert p4b = {x0, 0, z1, u0, vv1,  0, -1, 0};
            out.push_back(p1b); out.push_back(p4b); out.push_back(p2b);
            out.push_back(p1b); out.push_back(p3b); out.push_back(p4b);
        }
    }
}
