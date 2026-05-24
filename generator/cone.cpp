#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateCone(float radius, float height, int slices, int stacks, VertList& out) {
    const float PI = M_PI;
    float sliceStep = 2.0f * PI / slices;
    float stackStep = height / stacks;

    // Precompute slant normal scale factor
    float L = sqrtf(height * height + radius * radius);
    float nScale = 1.0f / L;

    // ========== BASE DISK (y=0) ==========
    // ✅ MUDANÇA: Usa cylindrical, não polar disk
    // u = angular (0 a 1), v = 0 (constante, na base)
    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;
        float u1 = 1.0f - (float)i / slices;
        float u2 = 1.0f - (float)(i + 1) / slices;

        float xb1 = radius * cosf(theta1), zb1 = radius * sinf(theta1);
        float xb2 = radius * cosf(theta2), zb2 = radius * sinf(theta2);
        float nx1 = cosf(theta1), nz1 = sinf(theta1);
        float nx2 = cosf(theta2), nz2 = sinf(theta2);

        // original: generateTriangle(xb1,0,zb1, xb2,0,zb2, 0,0,0)
        Vert vb1 = {xb1, 0, zb1, u1, 0.0f, nx1, 0, nz1};          // rim
        Vert vb2 = {xb2, 0, zb2, u2, 0.0f, nx2, 0, nz2};          // rim
        Vert vbc = {0, 0, 0, (u1 + u2) * 0.5f, 0.0f, 0, -1, 0};   // center
        out.push_back(vb1); out.push_back(vb2); out.push_back(vbc);
    }

    // ========== LATERAL SURFACE ==========
    // u wraps around (0 a 1), v goes bottom to top (0 a 1)
    for (int j = 0; j < stacks; j++) {
        float y_low  = j * stackStep;
        float y_high = (j + 1) * stackStep;
        float r_low  = radius * (1.0f - (float)j / stacks);
        float r_high = radius * (1.0f - (float)(j + 1) / stacks);
        float vv_low  = (float)j / stacks;
        float vv_high = (float)(j + 1) / stacks;

        for (int i = 0; i < slices; i++) {
            float theta1 = i * sliceStep;
            float theta2 = (i + 1) * sliceStep;
            float u1     = 1.0f - (float)i / slices;
            float u2     = 1.0f - (float)(i + 1) / slices;

            // Outward slant normal: n = normalize(h·cosθ, r, h·sinθ)
            auto makeLatNorm = [&](float theta) -> Vert {
                return {0, 0, 0, 0, 0,
                        height * cosf(theta) * nScale,
                        radius * nScale,
                        height * sinf(theta) * nScale};
            };

            if (j == stacks - 1) {
                // Top cap: triangle to apex
                float x1 = r_low * cosf(theta1), z1 = r_low * sinf(theta1);
                float x2 = r_low * cosf(theta2), z2 = r_low * sinf(theta2);
                float u_mid = (u1 + u2) * 0.5f;

                auto n1 = makeLatNorm(theta1);
                auto n2 = makeLatNorm(theta2);
                auto nm = makeLatNorm((theta1 + theta2) * 0.5f);

                Vert apex  = {0, height, 0, u_mid, 1.0f, nm.nx, nm.ny, nm.nz};
                Vert vlo2  = {x2, y_low, z2, u2, vv_low, n2.nx, n2.ny, n2.nz};
                Vert vlo1  = {x1, y_low, z1, u1, vv_low, n1.nx, n1.ny, n1.nz};
                out.push_back(apex); out.push_back(vlo2); out.push_back(vlo1);
            } else {
                float x1l = r_low  * cosf(theta1), z1l = r_low  * sinf(theta1);
                float x2l = r_low  * cosf(theta2), z2l = r_low  * sinf(theta2);
                float x1h = r_high * cosf(theta1), z1h = r_high * sinf(theta1);
                float x2h = r_high * cosf(theta2), z2h = r_high * sinf(theta2);

                auto n1 = makeLatNorm(theta1);
                auto n2 = makeLatNorm(theta2);

                Vert p1 = {x1l, y_low,  z1l, u1, vv_low,  n1.nx, n1.ny, n1.nz};
                Vert p2 = {x2l, y_low,  z2l, u2, vv_low,  n2.nx, n2.ny, n2.nz};
                Vert p3 = {x1h, y_high, z1h, u1, vv_high, n1.nx, n1.ny, n1.nz};
                Vert p4 = {x2h, y_high, z2h, u2, vv_high, n2.nx, n2.ny, n2.nz};
                out.push_back(p1); out.push_back(p4); out.push_back(p2);
                out.push_back(p1); out.push_back(p3); out.push_back(p4);
            }
        }
    }
}