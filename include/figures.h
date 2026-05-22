#pragma once
#include <string>
#include <list>
#include <vector>
#include "../include/generator_helpers.h"

void generateBox(float length, int divisions, VertList& out);
void generatePlane(float length, int divisions, VertList& out);
void generateSphere(float radius, int slices, int stacks, VertList& out);
void generateCone(float radius, float height, int slices, int stacks, VertList& out);
void generateCylinder(float radius, float height, int slices, int stacks, VertList& out);
void generateIcosphere(float radius, int subdivisions, VertList& out);
void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, VertList& out);
void generateRing(float innerRadius, float outerRadius, int slices, VertList& out);
void generateOctahedron(VertList& out, float x, float y, float z, float scale);
void generateBezier(const string& patchFile, int tessellation, VertList& out);
