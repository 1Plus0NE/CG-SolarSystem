#include "../include/generator_helpers.h"
#include "../include/figures.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Helper functions
void addVertex(list<string>& vertices, float x, float y, float z);
void generateTriangle(list<string>& vertices,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float x3, float y3, float z3);
void generateQuad(list<string>& vertices, 
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3,
                    float x4, float y4, float z4);
bool verifyMetric(const string& name, float value, float min);

// File I/O
void writeOutput(const list<string>& vertices, const string& file);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Adds a vertex to the vertices list
 */
void addVertex(list<string>& vertices, float x, float y, float z) {
    vertices.push_back(to_string(x) + " " + to_string(y) + " " + to_string(z));
}

/**
 * Generates a single triangle from 3 vertices
 * @param vertices - Output list of vertices
 * @param x1,y1,z1 - First vertex
 * @param x2,y2,z2 - Second vertex
 * @param x3,y3,z3 - Third vertex
 */
void generateTriangle(list<string>& vertices,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float x3, float y3, float z3) {
    addVertex(vertices, x1, y1, z1);
    addVertex(vertices, x2, y2, z2);
    addVertex(vertices, x3, y3, z3);
}

/**
 * Generates 2 triangles to form a quad in counter-clockwise order
 * p3 --- p4
 * |   /  |
 * |  /   |
 * p1 --- p2
 * 
 * Triangle 1: p1 -> p2 -> p4 (CCW)
 * Triangle 2: p1 -> p4 -> p3 (CCW)
 */
void generateQuad(list<string>& vertices, 
                    float x1, float y1, float z1,  // p1
                    float x2, float y2, float z2,  // p2
                    float x3, float y3, float z3,  // p3
                    float x4, float y4, float z4) { // p4

                        
    // Triangle 1: p1 -> p2 -> p4 (CCW)
    generateTriangle(vertices, x1, y1, z1, x2, y2, z2, x4, y4, z4);
    
    // Triangle 2: p1 -> p4 -> p3 (CCW)
    generateTriangle(vertices, x1, y1, z1, x4, y4, z4, x3, y3, z3);
}

/**
 * Verifies that a metric value meets the minimum requirement
 * @param name - Name of the metric (for error message)
 * @param value - Value to check
 * @param min - Minimum allowed value
 * @return true if valid, false otherwise
 */
bool verifyMetric(const string& name, float value, float min) {
    if (value < min) {
        cerr << "Error: " << name << " must be at least " << min 
             << " (got " << value << ")" << endl;
        return false;
    }
    return true;
}

// ============================================================================
// FILE I/O
// ============================================================================

/**
 * Writes list of vertices to a file in the figures directory
 * @param vertices - List of vertices to write
 * @param file - Output filename
 */
void writeOutput(const list<string>& vertices, const string& file) {
    // Build full path: ../figures/filename.3d
    string outputPath = "../../figures/" + file;

    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << outputPath << endl;
        cerr << "Make sure the 'figures' directory exists!" << endl;
        return;
    }

    // Write vertex count as first line for validation
    outFile << vertices.size() << endl;

    for (const auto& vertex : vertices) {
        outFile << vertex << endl;
    }

    outFile.close();
    cout << "Figure generated successfully: " << outputPath << endl;
    cout << "Total: " << vertices.size() << " vertices (" 
         << vertices.size() / 3 << " triangles)" << endl;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]){
    // Check arguments
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <shape> <parameters...> <output_file>" << endl;
        cerr << "Available shapes:" << endl;
        cerr << "  sphere <radius> <slices> <stacks> <output_file>" << endl;
        cerr << "  box <length> <divisions> <output_file>" << endl;
        cerr << "  cone <radius> <height> <slices> <stacks> <output_file>" << endl;
        cerr << "  plane <length> <divisions> <output_file>" << endl;
        cerr << "  cylinder <radius> <height> <slices> <stacks> <output_file>" << endl;
        cerr << "  icosphere <radius> <subdivisions> <output_file>" << endl;
        cerr << "  torus <R> <r> <slices> <stacks> <output_file>" << endl;
        cerr << "  ring <innerRadius> <outerRadius> <slices> <output_file>" << endl;
        return 1;
    }

    // Parse arguments
    list<string> arglist;
    string figure = argv[1];
    string file = argv[argc - 1];

    // Split arguments
    for (int i = 2; i < argc-1; i++) {
        arglist.push_back(argv[i]);
    }

    list<string> vertices;

    // Generate the requested shape
    if (figure == "sphere") {
        if (arglist.size() != 3) {
            cerr << "Usage: sphere <radius> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        arglist.pop_front();
        int stacks = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("slices", slices, 1) ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateSphere(radius, slices, stacks, vertices);
    } 
    else if (figure == "box") {
        if (arglist.size() != 2) {
            cerr << "Usage: box <length> <divisions> <output_file>" << endl;
            return 1;
        }
        float length = stof(arglist.front());
        arglist.pop_front();
        int divisions = stoi(arglist.front());
        if (!verifyMetric("length", length, 0.01) ||
            !verifyMetric("divisions", divisions, 1)) return 1;
        generateBox(length, divisions, vertices);
    } 
    else if (figure == "cone") {
        if (arglist.size() != 4) {
            cerr << "Usage: cone <radius> <height> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        float height = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        arglist.pop_front();
        int stacks = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("height", height, 0.01) ||
            !verifyMetric("slices", slices, 1) ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateCone(radius, height, slices, stacks, vertices);
    } 
    else if (figure == "plane") {
        if (arglist.size() != 2) {
            cerr << "Usage: plane <length> <divisions> <output_file>" << endl;
            return 1;
        }
        float length = stof(arglist.front());
        arglist.pop_front();
        int divisions = stoi(arglist.front());
        if (!verifyMetric("length", length, 0.01) ||
            !verifyMetric("divisions", divisions, 1)) return 1;
        generatePlane(length, divisions, vertices);
    } 
    else if (figure == "cylinder") {
        if (arglist.size() != 4) {
            cerr << "Usage: cylinder <radius> <height> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        float height = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        arglist.pop_front();
        int stacks = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("height", height, 0.01) ||
            !verifyMetric("slices", slices, 1) ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateCylinder(radius, height, slices, stacks, vertices);
    }
    else if (figure == "icosphere") {
        if (arglist.size() != 2) {
            cerr << "Usage: icosphere <radius> <subdivisions> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        int subdivisions = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("subdivisions", subdivisions, 0)) return 1;
        generateIcosphere(radius, subdivisions, vertices);

        
    }
    else if (figure == "torus") {
        if (arglist.size() != 4) {
            cerr << "Usage: torus <R> <r> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float R = stof(arglist.front()); arglist.pop_front();
        float r = stof(arglist.front()); arglist.pop_front();
        int slices = stoi(arglist.front()); arglist.pop_front();
        int stacks = stoi(arglist.front()); arglist.pop_front();
        
        generateTorus(R, r, slices, stacks, vertices);
    }
    else if (figure == "ring") {
        if (arglist.size() != 3) {
            cerr << "Usage: ring <innerRadius> <outerRadius> <slices> <output_file>" << endl;
            return 1;
        }
        float innerR = stof(arglist.front()); arglist.pop_front();
        float outerR = stof(arglist.front()); arglist.pop_front();
        int slices = stoi(arglist.front());
        if (!verifyMetric("innerRadius", innerR, 0.0f) ||
            !verifyMetric("outerRadius", outerR, 0.01f) ||
            !verifyMetric("slices", slices, 3)) return 1;
        generateRing(innerR, outerR, slices, vertices);
    }
    else {
        cerr << "Unknown figure type: " << figure << endl;
        cerr << "Available shapes: sphere, box, cone, plane, cylinder, icosphere, torus" << endl;
        return 1;
    }

    writeOutput(vertices, file);

    return 0;
}



