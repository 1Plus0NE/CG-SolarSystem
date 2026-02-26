#include <list>
#include <string>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

struct Point3D {
    float x, y, z;
    Point3D(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
};

void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, list<string>& vertices) {
    float sliceStep = 2.0f * M_PI / slices;
    float stackStep = 2.0f * M_PI / stacks;
    for (int i = 0; i < slices; i++) {
        float u1 = i * sliceStep;
        float u2 = (i + 1) * sliceStep;
        for (int j = 0; j < stacks; j++) {
            float v1 = j * stackStep;
            float v2 = (j + 1) * stackStep;
            auto getPoint = [&](float u, float v) {
                float horizontalDist = ringRadius + pipeRadius * cos(v);
                float x = horizontalDist * cos(u);
                float y = pipeRadius * sin(v);
                float z = horizontalDist * sin(u);
                return Point3D(x, y, z);
            };
            Point3D p1 = getPoint(u1, v1);
            Point3D p2 = getPoint(u2, v1);
            Point3D p3 = getPoint(u1, v2);
            Point3D p4 = getPoint(u2, v2);
            generateQuad(vertices,
                p1.x, p1.y, p1.z,
                p2.x, p2.y, p2.z,
                p3.x, p3.y, p3.z,
                p4.x, p4.y, p4.z);
        }
    }
}
