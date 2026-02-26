#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;


void generatePlane(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f;
    float step = length / divisions;
    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float z0 = -half + j * step;
            float z1 = -half + (j + 1) * step;
            // Top face
            generateQuad(vertices,
                x0, 0.0f, z0,
                x1, 0.0f, z0,
                x0, 0.0f, z1,
                x1, 0.0f, z1);
            // Bottom face
            generateQuad(vertices,
                x1, 0.0f, z0,
                x0, 0.0f, z0,
                x1, 0.0f, z1,
                x0, 0.0f, z1);
        }
    }
}
