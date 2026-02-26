#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"
using namespace std;

void generateCylinder(float radius, float height, int slices, int stacks, list<string>& vertices) {
    float sliceStep = (2 * M_PI) / slices;
    float stackStep = height / stacks;

    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;
        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        // Bottom cap
        generateTriangle(vertices, x1, 0, z1, x2, 0, z2, 0, 0, 0);

        // Top cap
        generateTriangle(vertices, x1, height, z1, 0, height, 0, x2, height, z2);
    }

    // Side wall subdivided by stacks
    for (int j = 0; j < stacks; j++) {
        float y_low  = j * stackStep;
        float y_high = (j + 1) * stackStep;

        for (int i = 0; i < slices; i++) {
            float theta1 = i * sliceStep;
            float theta2 = (i + 1) * sliceStep;
            float x1 = radius * cos(theta1);
            float z1 = radius * sin(theta1);
            float x2 = radius * cos(theta2);
            float z2 = radius * sin(theta2);

            generateQuad(vertices,
                x2, y_low,  z2,
                x1, y_low,  z1,
                x2, y_high, z2,
                x1, y_high, z1);
        }
    }
}
