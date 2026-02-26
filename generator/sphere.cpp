#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateSphere(float radius, int slices, int stacks, list<string>& vertices) {
    const float PI = M_PI;
    float stackStep = PI / stacks;
    float sliceStep = 2 * PI / slices;
    for (int i = 0; i < stacks; i++) {
        float phi1 = i * stackStep;
        float phi2 = (i + 1) * stackStep;
        for (int j = 0; j < slices; j++) {
            float theta1 = j * sliceStep;
            float theta2 = (j + 1) * sliceStep;
            if (i == 0) {
                float x2_1 = radius * sin(phi2) * cos(theta1);
                float y2_1 = radius * cos(phi2);
                float z2_1 = radius * sin(phi2) * sin(theta1);
                float x2_2 = radius * sin(phi2) * cos(theta2);
                float y2_2 = radius * cos(phi2);
                float z2_2 = radius * sin(phi2) * sin(theta2);
                generateTriangle(vertices,
                    0, radius, 0,
                    x2_2, y2_2, z2_2,
                    x2_1, y2_1, z2_1);
            } else if (i == stacks - 1) {
                float x1_1 = radius * sin(phi1) * cos(theta1);
                float y1_1 = radius * cos(phi1);
                float z1_1 = radius * sin(phi1) * sin(theta1);
                float x1_2 = radius * sin(phi1) * cos(theta2);
                float y1_2 = radius * cos(phi1);
                float z1_2 = radius * sin(phi1) * sin(theta2);
                generateTriangle(vertices,
                    x1_1, y1_1, z1_1,
                    x1_2, y1_2, z1_2,
                    0, -radius, 0);
            } else {
                float x1_1 = radius * sin(phi1) * cos(theta1);
                float y1_1 = radius * cos(phi1);
                float z1_1 = radius * sin(phi1) * sin(theta1);
                float x1_2 = radius * sin(phi1) * cos(theta2);
                float y1_2 = radius * cos(phi1);
                float z1_2 = radius * sin(phi1) * sin(theta2);
                float x2_1 = radius * sin(phi2) * cos(theta1);
                float y2_1 = radius * cos(phi2);
                float z2_1 = radius * sin(phi2) * sin(theta1);
                float x2_2 = radius * sin(phi2) * cos(theta2);
                float y2_2 = radius * cos(phi2);
                float z2_2 = radius * sin(phi2) * sin(theta2);
                generateQuad(vertices,
                    x1_1, y1_1, z1_1,
                    x1_2, y1_2, z1_2,
                    x2_1, y2_1, z2_1,
                    x2_2, y2_2, z2_2);
            }
        }
    }
}
