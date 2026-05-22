#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

// Pushes a quad (two CCW triangles) given 4 vertices in p1,p2,p3,p4 order
// where generateQuad logic is tri(p1,p2,p4) + tri(p1,p4,p3).
static void pushQuad(VertList& out,
                     const Vert& p1, const Vert& p2,
                     const Vert& p3, const Vert& p4) {
    out.push_back(p1); out.push_back(p2); out.push_back(p4);
    out.push_back(p1); out.push_back(p4); out.push_back(p3);
}

void generateBox(float length, int divisions, VertList& out) {
    float half = length / 2.0f;
    float step = length / divisions;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float a0 = -half + i * step,      a1 = -half + (i + 1) * step;
            float b0 = -half + j * step,      b1 = -half + (j + 1) * step;
            float u0 = (float)i / divisions,  u1 = (float)(i + 1) / divisions;
            float v0 = (float)j / divisions,  v1 = (float)(j + 1) / divisions;

            // Front face (z = +half), normal (0, 0, +1)
            // x varies with i (a), y varies with j (b)
            pushQuad(out,
                {a0, b0, half, u0, v0, 0, 0, 1},
                {a1, b0, half, u1, v0, 0, 0, 1},
                {a0, b1, half, u0, v1, 0, 0, 1},
                {a1, b1, half, u1, v1, 0, 0, 1});

            // Back face (z = -half), normal (0, 0, -1), u mirrored
            pushQuad(out,
                {a1, b0, -half, 1.0f - u1, v0, 0, 0, -1},
                {a0, b0, -half, 1.0f - u0, v0, 0, 0, -1},
                {a1, b1, -half, 1.0f - u1, v1, 0, 0, -1},
                {a0, b1, -half, 1.0f - u0, v1, 0, 0, -1});

            // Right face (x = +half), normal (+1, 0, 0)
            // original uses z=a1..a0 (reversed), y=b0..b1
            pushQuad(out,
                {half, b0, a1, 1.0f - u1, v0, 1, 0, 0},
                {half, b0, a0, 1.0f - u0, v0, 1, 0, 0},
                {half, b1, a1, 1.0f - u1, v1, 1, 0, 0},
                {half, b1, a0, 1.0f - u0, v1, 1, 0, 0});

            // Left face (x = -half), normal (-1, 0, 0)
            // original uses z=a0..a1, y=b0..b1
            pushQuad(out,
                {-half, b0, a0, u0, v0, -1, 0, 0},
                {-half, b0, a1, u1, v0, -1, 0, 0},
                {-half, b1, a0, u0, v1, -1, 0, 0},
                {-half, b1, a1, u1, v1, -1, 0, 0});

            // Top face (y = +half), normal (0, +1, 0)
            // original uses x=a0..a1, z=b1..b0 (v mirrored)
            pushQuad(out,
                {a0, half, b1, u0, 1.0f - v1, 0, 1, 0},
                {a1, half, b1, u1, 1.0f - v1, 0, 1, 0},
                {a0, half, b0, u0, 1.0f - v0, 0, 1, 0},
                {a1, half, b0, u1, 1.0f - v0, 0, 1, 0});

            // Bottom face (y = -half), normal (0, -1, 0)
            pushQuad(out,
                {a0, -half, b0, u0, v0, 0, -1, 0},
                {a1, -half, b0, u1, v0, 0, -1, 0},
                {a0, -half, b1, u0, v1, 0, -1, 0},
                {a1, -half, b1, u1, v1, 0, -1, 0});
        }
    }
}
