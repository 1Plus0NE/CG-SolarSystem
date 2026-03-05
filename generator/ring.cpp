#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

/**
 * Generates a flat annular ring (disk with a hole) in the XZ plane.
 * Useful for Saturn's rings.
 *
 * @param innerRadius  Inner radius of the ring
 * @param outerRadius  Outer radius of the ring
 * @param slices       Number of angular divisions around the ring
 * @param vertices     Output vertex list
 *
 * Creates two triangle fans (top and bottom faces) to keep backface culling happy.
 * Each slice is a quad between innerRadius and outerRadius at angle [a, a+step].
 *
 *   outer       outer
 *    p2 ------- p4        (top face CCW from above: p1 p2 p4, p1 p4 p3)
 *    |           |
 *    p1 ------- p3
 *   inner       inner
 */
void generateRing(float innerRadius, float outerRadius, int slices, list<string>& vertices) {
    float step = 2.0f * M_PI / slices;

    for (int i = 0; i < slices; i++) {
        float a1 = i * step;
        float a2 = (i + 1) * step;

        float ix1 = innerRadius * cos(a1), iz1 = innerRadius * sin(a1);
        float ox1 = outerRadius * cos(a1), oz1 = outerRadius * sin(a1);
        float ix2 = innerRadius * cos(a2), iz2 = innerRadius * sin(a2);
        float ox2 = outerRadius * cos(a2), oz2 = outerRadius * sin(a2);

        // p1=inner a1, p2=outer a1, p3=inner a2, p4=outer a2  (all at y=0)
        // Top face (normal +Y): CCW from above = p1, p2, p4, p1, p4, p3
        generateQuad(vertices,
            ix1, 0.0f, iz1,   // p1 inner a1
            ox1, 0.0f, oz1,   // p2 outer a1
            ix2, 0.0f, iz2,   // p3 inner a2
            ox2, 0.0f, oz2);  // p4 outer a2

        // Bottom face (normal -Y): reverse winding
        generateQuad(vertices,
            ox1, 0.0f, oz1,   // p2
            ix1, 0.0f, iz1,   // p1
            ox2, 0.0f, oz2,   // p4
            ix2, 0.0f, iz2);  // p3
    }
}
