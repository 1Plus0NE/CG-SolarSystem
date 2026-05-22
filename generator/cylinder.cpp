#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateCylinder(float radius, float height, int slices, int stacks, VertList& out) {
    const float PI = M_PI;

    // ========== BOTTOM CAP (y=0) ==========
    // ✅ MUDANÇA: Usa cylindrical, não polar disk
    // u = angular (0 a 1), v = 0 (constante, no fundo)
    for (int i = 0; i < slices; i++) {
        float theta1 = (float)i       / slices * 2.0f * PI;
        float theta2 = (float)(i + 1) / slices * 2.0f * PI;
        float u1 = (float)i       / slices;
        float u2 = (float)(i + 1) / slices;
        
        float x1 = radius * cosf(theta1), z1 = radius * sinf(theta1);
        float x2 = radius * cosf(theta2), z2 = radius * sinf(theta2);
        float nx1 = cosf(theta1), nz1 = sinf(theta1);
        float nx2 = cosf(theta2), nz2 = sinf(theta2);

        // Bottom cap: triangle fan from center
        // original: generateTriangle(x1,0,z1, x2,0,z2, 0,0,0)
        Vert vb1  = {x1, 0, z1, u1, 0.0f, nx1, 0, nz1};          // rim v1
        Vert vb2  = {x2, 0, z2, u2, 0.0f, nx2, 0, nz2};          // rim v2
        Vert vbc  = {0, 0, 0, (u1 + u2) * 0.5f, 0.0f, 0, -1, 0}; // center
        out.push_back(vb1); out.push_back(vb2); out.push_back(vbc);
    }

    // ========== TOP CAP (y=height) ==========
    // ✅ MUDANÇA: Usa cylindrical, não polar disk
    // u = angular (0 a 1), v = 1 (constante, no topo)
    for (int i = 0; i < slices; i++) {
        float theta1 = (float)i       / slices * 2.0f * PI;
        float theta2 = (float)(i + 1) / slices * 2.0f * PI;
        float u1 = (float)i       / slices;
        float u2 = (float)(i + 1) / slices;
        
        float x1 = radius * cosf(theta1), z1 = radius * sinf(theta1);
        float x2 = radius * cosf(theta2), z2 = radius * sinf(theta2);
        float nx1 = cosf(theta1), nz1 = sinf(theta1);
        float nx2 = cosf(theta2), nz2 = sinf(theta2);

        // Top cap: triangle fan from center (normal reversed)
        // original: generateTriangle(x1,h,z1, 0,h,0, x2,h,z2)
        Vert vt1  = {x1, height, z1, u1, 1.0f, nx1, 0, nz1};          // rim v1
        Vert vtc  = {0, height, 0, (u1 + u2) * 0.5f, 1.0f, 0, 1, 0}; // center
        Vert vt2  = {x2, height, z2, u2, 1.0f, nx2, 0, nz2};          // rim v2
        out.push_back(vt1); out.push_back(vtc); out.push_back(vt2);
    }

    // ========== LATERAL SURFACE (já estava bem) ==========
    // u wraps around (0 a 1), v goes bottom to top (0 a 1)
    for (int j = 0; j < stacks; j++) {
        float y_low  = (float)j       / stacks * height;
        float y_high = (float)(j + 1) / stacks * height;
        float vv_low  = (float)j       / stacks;
        float vv_high = (float)(j + 1) / stacks;

        for (int i = 0; i < slices; i++) {
            float theta1 = (float)i       / slices * 2.0f * PI;
            float theta2 = (float)(i + 1) / slices * 2.0f * PI;
            float u1 = (float)i       / slices;
            float u2 = (float)(i + 1) / slices;
            float x1 = radius * cosf(theta1), z1 = radius * sinf(theta1);
            float x2 = radius * cosf(theta2), z2 = radius * sinf(theta2);
            float nx1 = cosf(theta1), nz1 = sinf(theta1);
            float nx2 = cosf(theta2), nz2 = sinf(theta2);

            Vert p1 = {x2, y_low,  z2, u2, vv_low,  nx2, 0, nz2};
            Vert p2 = {x1, y_low,  z1, u1, vv_low,  nx1, 0, nz1};
            Vert p3 = {x2, y_high, z2, u2, vv_high, nx2, 0, nz2};
            Vert p4 = {x1, y_high, z1, u1, vv_high, nx1, 0, nz1};
            out.push_back(p1); out.push_back(p2); out.push_back(p4);
            out.push_back(p1); out.push_back(p4); out.push_back(p3);
        }
    }
}