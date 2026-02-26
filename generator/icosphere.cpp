#include <list>
#include <string>
#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;


struct Point3D {
    float x, y, z;
    Point3D(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    Point3D normalize(float radius) const {
        float length = sqrt(x*x + y*y + z*z);
        return Point3D(x * radius / length, y * radius / length, z * radius / length);
    }
};

void generateIcosphere(float radius, int subdivisions, list<string>& vertices) {
    const float t = (1.0f + sqrt(5.0f)) / 2.0f;
    vector<Point3D> baseVertices = {
        Point3D(-1,  t,  0).normalize(radius),
        Point3D( 1,  t,  0).normalize(radius),
        Point3D(-1, -t,  0).normalize(radius),
        Point3D( 1, -t,  0).normalize(radius),
        Point3D( 0, -1,  t).normalize(radius),
        Point3D( 0,  1,  t).normalize(radius),
        Point3D( 0, -1, -t).normalize(radius),
        Point3D( 0,  1, -t).normalize(radius),
        Point3D( t,  0, -1).normalize(radius),
        Point3D( t,  0,  1).normalize(radius),
        Point3D(-t,  0, -1).normalize(radius),
        Point3D(-t,  0,  1).normalize(radius)
    };
    struct TriangleIndex { int v1, v2, v3; };
    vector<TriangleIndex> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };
    vector<Point3D> currentVertices = baseVertices;
    auto getMiddlePoint = [&](int p1, int p2) -> int {
        Point3D point1 = currentVertices[p1];
        Point3D point2 = currentVertices[p2];
        Point3D middle(
            (point1.x + point2.x) / 2.0f,
            (point1.y + point2.y) / 2.0f,
            (point1.z + point2.z) / 2.0f
        );
        currentVertices.push_back(middle.normalize(radius));
        return currentVertices.size() - 1;
    };
    for(int i = 0; i < subdivisions; i++) {
        vector<TriangleIndex> nextFaces;
        for(const auto& face : faces) {
            int a = getMiddlePoint(face.v1, face.v2);
            int b = getMiddlePoint(face.v2, face.v3);
            int c = getMiddlePoint(face.v3, face.v1);
            nextFaces.push_back({face.v1, a, c});
            nextFaces.push_back({face.v2, b, a});
            nextFaces.push_back({face.v3, c, b});
            nextFaces.push_back({a, b, c});
        }
        faces = nextFaces;
    }
    for(const auto& face : faces) {
        generateTriangle(vertices,
            currentVertices[face.v1].x, currentVertices[face.v1].y, currentVertices[face.v1].z,
            currentVertices[face.v2].x, currentVertices[face.v2].y, currentVertices[face.v2].z,
            currentVertices[face.v3].x, currentVertices[face.v3].y, currentVertices[face.v3].z);
    }
}
