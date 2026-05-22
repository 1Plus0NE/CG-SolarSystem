#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateSphere(float radius, int slices, int stacks, VertList& out) {
    const float PI = M_PI;

    // Returns a vertex on the sphere surface given spherical coordinates.
    // Normal = normalized position. UV uses standard cylindrical projection:
    //   u = longitude / 2π  (0 at +X, increases eastward)
    //   v = 1 - colatitude/π  (1 at north pole, 0 at south pole)
    auto makeVert = [&](float phi, float theta, float u, float v) -> Vert {
        float nx = sinf(phi) * cosf(theta);
        float ny = cosf(phi);
        float nz = sinf(phi) * sinf(theta);
        return {radius * nx, radius * ny, radius * nz, u, v, nx, ny, nz};
    };

    for (int i = 0; i < stacks; i++) {
        float phi1 = (float)i       / stacks * PI;
        float phi2 = (float)(i + 1) / stacks * PI;
        float v1   = 1.0f - (float)i       / stacks;
        float v2   = 1.0f - (float)(i + 1) / stacks;

        for (int j = 0; j < slices; j++) {
            float theta1 = (float)j       / slices * 2.0f * PI;
            float theta2 = (float)(j + 1) / slices * 2.0f * PI;
            float u1     = (float)j       / slices;
            float u2     = (float)(j + 1) / slices;

            if (i == 0) {
                // Top pole: apex u is the midpoint between the two slice edges
                Vert top = {0, radius, 0, (u1 + u2) * 0.5f, 1.0f, 0, 1, 0};
                Vert bl  = makeVert(phi2, theta1, u1, v2);
                Vert br  = makeVert(phi2, theta2, u2, v2);
                out.push_back(top);
                out.push_back(br);
                out.push_back(bl);
            } else if (i == stacks - 1) {
                // Bottom pole: apex u is the midpoint
                Vert tl  = makeVert(phi1, theta1, u1, v1);
                Vert tr  = makeVert(phi1, theta2, u2, v1);
                Vert bot = {0, -radius, 0, (u1 + u2) * 0.5f, 0.0f, 0, -1, 0};
                out.push_back(tl);
                out.push_back(tr);
                out.push_back(bot);
            } else {
                Vert tl = makeVert(phi1, theta1, u1, v1);
                Vert tr = makeVert(phi1, theta2, u2, v1);
                Vert bl = makeVert(phi2, theta1, u1, v2);
                Vert br = makeVert(phi2, theta2, u2, v2);
                out.push_back(tl); out.push_back(tr); out.push_back(br);
                out.push_back(tl); out.push_back(br); out.push_back(bl);
            }
        }
    }
}
