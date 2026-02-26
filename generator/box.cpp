#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

void generateBox(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f;
    float step = length / divisions;
    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float y0 = -half + j * step;
            float y1 = -half + (j + 1) * step;
            // Front face
            generateQuad(vertices,
                x0, y0, half,
                x1, y0, half,
                x0, y1, half,
                x1, y1, half);
            // Back face
            generateQuad(vertices,
                x1, y0, -half,
                x0, y0, -half,
                x1, y1, -half,
                x0, y1, -half);
            // Right face
            generateQuad(vertices,
                half, y0, x1,
                half, y0, x0,
                half, y1, x1,
                half, y1, x0);
            // Left face
            generateQuad(vertices,
                -half, y0, x0,
                -half, y0, x1,
                -half, y1, x0,
                -half, y1, x1);
            // Top face
            generateQuad(vertices,
                x0, half, y1,
                x1, half, y1,
                x0, half, y0,
                x1, half, y0);
            // Bottom face
            generateQuad(vertices,
                x0, -half, y0,
                x1, -half, y0,
                x0, -half, y1,
                x1, -half, y1);
        }
    }
}
