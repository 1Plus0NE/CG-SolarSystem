#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"
using namespace std;

void generateCylinder(float radius, float height, int slices, list<string>& vertices) {
    float sliceStep = (2 * M_PI) / slices;
    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;
        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);
        generateTriangle(vertices, x1, 0, z1, x2, 0, z2, 0, 0, 0);
        generateTriangle(vertices, x1, height, z1, 0, height, 0, x2, height, z2);
        generateQuad(vertices,
            x2, 0,      z2,
            x1, 0,      z1,
            x2, height, z2,
            x1, height, z1);
    }
}
