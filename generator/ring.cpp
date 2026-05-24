#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

// Flat annular ring in the XZ plane.
// UV: u = angular fraction θ/2π, v = radial fraction (r−inner)/(outer−inner)
//   v=0 at inner edge, v=1 at outer edge — natural for a ring texture strip.
void generateRing(float innerRadius, float outerRadius, int slices, VertList& out) {
    const float PI = M_PI;
    float step = 2.0f * PI / slices;
    float radSpan = outerRadius - innerRadius;

    for (int i = 0; i < slices; i++) {
        float a1 = i * step;
        float a2 = (i + 1) * step;
        float u1 = 1.0f - (float)i       / slices;
        float u2 = 1.0f - (float)(i + 1) / slices;

        float ix1 = innerRadius * cosf(a1), iz1 = innerRadius * sinf(a1);
        float ox1 = outerRadius * cosf(a1), oz1 = outerRadius * sinf(a1);
        float ix2 = innerRadius * cosf(a2), iz2 = innerRadius * sinf(a2);
        float ox2 = outerRadius * cosf(a2), oz2 = outerRadius * sinf(a2);

        (void)radSpan;

        // p1=inner_a1  p2=outer_a1  p3=inner_a2  p4=outer_a2
        // Top face (normal +Y): original generateQuad(p1,p2,p3,p4)
        Vert tp1 = {ix1, 0, iz1, u1, 0.0f, 0, 1, 0};
        Vert tp2 = {ox1, 0, oz1, u1, 1.0f, 0, 1, 0};
        Vert tp3 = {ix2, 0, iz2, u2, 0.0f, 0, 1, 0};
        Vert tp4 = {ox2, 0, oz2, u2, 1.0f, 0, 1, 0};
        out.push_back(tp1); out.push_back(tp2); out.push_back(tp4);
        out.push_back(tp1); out.push_back(tp4); out.push_back(tp3);

        // Bottom face (normal -Y): reversed winding
        // original generateQuad(p2,p1,p4,p3)
        Vert bp1 = {ox1, 0, oz1, u1, 1.0f, 0, -1, 0};
        Vert bp2 = {ix1, 0, iz1, u1, 0.0f, 0, -1, 0};
        Vert bp3 = {ox2, 0, oz2, u2, 1.0f, 0, -1, 0};
        Vert bp4 = {ix2, 0, iz2, u2, 0.0f, 0, -1, 0};
        out.push_back(bp1); out.push_back(bp2); out.push_back(bp4);
        out.push_back(bp1); out.push_back(bp4); out.push_back(bp3);
    }
}
