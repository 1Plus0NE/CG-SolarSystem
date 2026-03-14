#pragma once
#include <list>
#include <string>
#include <vector> // Added to fix missing vector type
using namespace std;

void generateBox(float length, int divisions, list<string>& vertices);
void generatePlane(float length, int divisions, list<string>& vertices);
void generateSphere(float radius, int slices, int stacks, list<string>& vertices);
void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices);
void generateCylinder(float radius, float height, int slices, int stacks, list<string>& vertices);
void generateIcosphere(float radius, int subdivisions, list<string>& vertices);
void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, list<string>& vertices);
void generateRing(float innerRadius, float outerRadius, int slices, list<string>& vertices);
void generateOctahedron(list<string>& vertices, float x, float y, float z, float scale);
void generateScatter(const string& shape, const vector<float>& params,
                     const string& modelFile, float scaleMin, float scaleMax,
                     int num, list<string>& vertices);
