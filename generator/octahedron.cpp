#include <list>
#include <string>
#include "../include/generator_helpers.h"

using namespace std;

void generateOctahedron(list<string>& vertices, float x, float y, float z, float scale) {
    float v[6][3] = {
        {1*scale + x, 0*scale + y, 0*scale + z},
        {-1*scale + x, 0*scale + y, 0*scale + z},
        {0*scale + x, 1*scale + y, 0*scale + z},
        {0*scale + x, -1*scale + y, 0*scale + z},
        {0*scale + x, 0*scale + y, 1*scale + z},
        {0*scale + x, 0*scale + y, -1*scale + z}
    };
    generateTriangle(vertices, v[4][0], v[4][1], v[4][2], v[2][0], v[2][1], v[2][2], v[0][0], v[0][1], v[0][2]);
    generateTriangle(vertices, v[4][0], v[4][1], v[4][2], v[1][0], v[1][1], v[1][2], v[2][0], v[2][1], v[2][2]);
    generateTriangle(vertices, v[4][0], v[4][1], v[4][2], v[3][0], v[3][1], v[3][2], v[1][0], v[1][1], v[1][2]);
    generateTriangle(vertices, v[4][0], v[4][1], v[4][2], v[0][0], v[0][1], v[0][2], v[3][0], v[3][1], v[3][2]);
    generateTriangle(vertices, v[5][0], v[5][1], v[5][2], v[2][0], v[2][1], v[2][2], v[0][0], v[0][1], v[0][2]);
    generateTriangle(vertices, v[5][0], v[5][1], v[5][2], v[1][0], v[1][1], v[1][2], v[2][0], v[2][1], v[2][2]);
    generateTriangle(vertices, v[5][0], v[5][1], v[5][2], v[3][0], v[3][1], v[3][2], v[1][0], v[1][1], v[1][2]);
    generateTriangle(vertices, v[5][0], v[5][1], v[5][2], v[0][0], v[0][1], v[0][2], v[3][0], v[3][1], v[3][2]);
}