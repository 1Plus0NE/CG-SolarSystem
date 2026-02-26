#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"
using namespace std;

void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices) {
    float sliceStep = (2 * M_PI) / slices;
    float stackStep = height / stacks;
    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;
        float xb1 = radius * cos(theta1);
        float zb1 = radius * sin(theta1);
        float xb2 = radius * cos(theta2);
        float zb2 = radius * sin(theta2);
        generateTriangle(vertices, xb1, 0, zb1, xb2, 0, zb2, 0, 0, 0);
    }
    for (int j = 0; j < stacks; j++) {
        float y_low = j * stackStep;
        float y_high = (j + 1) * stackStep;
        float r_low  = radius * (1.0f - (float)j / stacks);
        float r_high = radius * (1.0f - (float)(j + 1) / stacks);
        for (int i = 0; i < slices; i++) {
            float theta1 = i * sliceStep;
            float theta2 = (i + 1) * sliceStep;
            float x1_low  = r_low  * cos(theta1);
            float z1_low  = r_low  * sin(theta1);
            float x2_low  = r_low  * cos(theta2);
            float z2_low  = r_low  * sin(theta2);
            float x1_high = r_high * cos(theta1);
            float z1_high = r_high * sin(theta1);
            float x2_high = r_high * cos(theta2);
            float z2_high = r_high * sin(theta2);
            if (j == stacks - 1) {
                generateTriangle(vertices, 0, height, 0, x2_low, y_low, z2_low, x1_low, y_low, z1_low);
            } else {
               generateQuad(vertices,
                            x1_low,  y_low,  z1_low,
                            x2_low,  y_low,  z2_low,
                            x1_high, y_high, z1_high,
                            x2_high, y_high, z2_high);
                            }
        }
    }
}
