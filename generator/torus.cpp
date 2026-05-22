#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, VertList& out) {
    const float PI = M_PI;

    // u = main ring angle / 2π, v = tube cross-section angle / 2π
    // Normal: outward from the tube axis = (cos(v)·cos(u), sin(v), cos(v)·sin(u))
    auto makeVert = [&](float u, float v) -> Vert {
        float theta = u * 2.0f * PI;
        float phi   = v * 2.0f * PI;
        float hd = ringRadius + pipeRadius * cosf(phi);
        float x = hd * cosf(theta);
        float y = pipeRadius * sinf(phi);
        float z = hd * sinf(theta);
        float nx = cosf(phi) * cosf(theta);
        float ny = sinf(phi);
        float nz = cosf(phi) * sinf(theta);
        return {x, y, z, u, v, nx, ny, nz};
    };

    for (int i = 0; i < slices; i++) {
        float u1 = (float)i       / slices;
        float u2 = (float)(i + 1) / slices;
        for (int j = 0; j < stacks; j++) {
            float v1 = (float)j       / stacks;
            float v2 = (float)(j + 1) / stacks;

            Vert p1 = makeVert(u1, v1);
            Vert p2 = makeVert(u2, v1);
            Vert p3 = makeVert(u1, v2);
            Vert p4 = makeVert(u2, v2);
            // Winding adjusted for outward-facing front faces with back-face culling
            out.push_back(p1); out.push_back(p4); out.push_back(p2);
            out.push_back(p1); out.push_back(p3); out.push_back(p4);
        }
    }
}
