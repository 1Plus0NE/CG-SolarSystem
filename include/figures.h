#pragma once
#include <list>
#include <string>
using namespace std;

void generateBox(float length, int divisions, list<string>& vertices);
void generatePlane(float length, int divisions, list<string>& vertices);
void generateSphere(float radius, int slices, int stacks, list<string>& vertices);
void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices);
void generateCylinder(float radius, float height, int slices, int stacks, list<string>& vertices);
void generateIcosphere(float radius, int subdivisions, list<string>& vertices);
void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, list<string>& vertices);
