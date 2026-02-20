#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <cmath>

using namespace std;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Helper functions
void addVertex(list<string>& vertices, float x, float y, float z);
void generateSquare(list<string>& vertices, 
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3,
                    float x4, float y4, float z4);
bool verifyMetric(const string& name, float value, float min);

// Shape generators
void generateBox(float length, int divisions, list<string>& vertices);
void generatePlane(float length, int divisions, list<string>& vertices);
void generateSphere(float radius, int slices, int stacks, list<string>& vertices);
void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices);

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
 * Generates 2 triangles to form a square in counter-clockwise order
 * p1 --- p2
 * |  \   |
 * |   \  |
 * p3 --- p4
 */
void generateSquare(list<string>& vertices, 
                    float x1, float y1, float z1,  // p1
                    float x2, float y2, float z2,  // p2
                    float x3, float y3, float z3,  // p3
                    float x4, float y4, float z4) { // p4
    // Triangle 1: p1, p2, p4
    addVertex(vertices, x1, y1, z1);
    addVertex(vertices, x2, y2, z2);
    addVertex(vertices, x4, y4, z4);
    
    // Triangle 2: p1, p4, p3
    addVertex(vertices, x1, y1, z1);
    addVertex(vertices, x4, y4, z4);
    addVertex(vertices, x3, y3, z3);
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
// SHAPE GENERATORS
// ============================================================================


/**
 * Generates a box centered at the origin with counter-clockwise faces
 * @param length - Side length of the box
 * @param divisions - Number of divisions per edge
 * @param vertices - Output list of vertices
 */
void generateBox(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f;  // To center at origin
    float step = length / divisions; // Distance between vertices

    // Generate all 6 faces of the cube in counter-clockwise order (CCW)
    // When viewed from OUTSIDE the cube, vertices must be in counter-clockwise order
    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float y0 = -half + j * step;
            float y1 = -half + (j + 1) * step;
            
            // Front face (z = +half) - looking from +Z towards origin
            // CCW when viewed from outside: bottom-left, bottom-right, top-left, top-right
            generateSquare(vertices,
                x0, y0, half,   // p1: bottom-left
                x1, y0, half,   // p2: bottom-right
                x0, y1, half,   // p3: top-left
                x1, y1, half);  // p4: top-right
            
            // Back face (z = -half) - looking from -Z towards origin
            // CCW when viewed from outside: bottom-right, bottom-left, top-right, top-left
            generateSquare(vertices,
                x1, y0, -half,  // p1: bottom-right
                x0, y0, -half,  // p2: bottom-left
                x1, y1, -half,  // p3: top-right
                x0, y1, -half); // p4: top-left
            
            // Right face (x = +half) - looking from +X towards origin
            // CCW when viewed from outside
            generateSquare(vertices,
                half, y0, x1,   // p1
                half, y0, x0,   // p2
                half, y1, x1,   // p3
                half, y1, x0);  // p4
            
            // Left face (x = -half) - looking from -X towards origin
            // CCW when viewed from outside
            generateSquare(vertices,
                -half, y0, x0,  // p1
                -half, y0, x1,  // p2
                -half, y1, x0,  // p3
                -half, y1, x1); // p4
            
            // Top face (y = +half) - looking from +Y downwards
            // CCW when viewed from outside
            generateSquare(vertices,
                x0, half, y1,   // p1
                x1, half, y1,   // p2
                x0, half, y0,   // p3
                x1, half, y0);  // p4
            
            // Bottom face (y = -half) - looking from -Y upwards
            // CCW when viewed from outside
            generateSquare(vertices,
                x0, -half, y0,  // p1
                x1, -half, y0,  // p2
                x0, -half, y1,  // p3
                x1, -half, y1); // p4
        }
    }
}

/**
 * Generates a plane centered at the origin
 * @param length - Side length of the plane
 * @param divisions - Number of divisions per edge
 * @param vertices - Output list of vertices
 */
void generatePlane(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f; // To center at origin
    float step = length / divisions; // Distance between vertices

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float z0 = -half + j * step;
            float z1 = -half + (j + 1) * step;

            // Top face - CCW when viewed from above (+Y)
            generateSquare(vertices,
                x0, 0.0f, z0,  // p1: bottom-left
                x1, 0.0f, z0,  // p2: bottom-right
                x0, 0.0f, z1,  // p3: top-left
                x1, 0.0f, z1); // p4: top-right

            // Bottom face - CCW when viewed from below (-Y)
            generateSquare(vertices,
                x1, 0.0f, z0,  // p1
                x0, 0.0f, z0,  // p2
                x1, 0.0f, z1,  // p3
                x0, 0.0f, z1); // p4
        }
    }
}

/**
 * Generates a sphere centered at the origin
 * @param radius - Radius of the sphere
 * @param slices - Number of vertical slices (longitude divisions)
 * @param stacks - Number of horizontal stacks (latitude divisions)
 * @param vertices - Output list of vertices
 */
void generateSphere(float radius, int slices, int stacks, list<string>& vertices) {
    const float PI = M_PI;
    
    // Preocumputation of divisions, this way it generates more efficiently
    float stackStep = PI / stacks;
    float sliceStep = 2 * PI / slices;

    for (int i = 0; i < stacks; i++) {
        float phi1 = i * stackStep; 
        float phi2 = (i + 1) * stackStep; 
        
        for (int j = 0; j < slices; j++) {
            float theta1 = j * sliceStep;
            float theta2 = (j + 1) * sliceStep;
            
            float x1_1 = radius * sin(phi1) * cos(theta1);
            float y1_1 = radius * cos(phi1);
            float z1_1 = radius * sin(phi1) * sin(theta1);
            
            float x1_2 = radius * sin(phi1) * cos(theta2);
            float y1_2 = radius * cos(phi1);
            float z1_2 = radius * sin(phi1) * sin(theta2);
            
            float x2_1 = radius * sin(phi2) * cos(theta1);
            float y2_1 = radius * cos(phi2);
            float z2_1 = radius * sin(phi2) * sin(theta1);
            
            float x2_2 = radius * sin(phi2) * cos(theta2);
            float y2_2 = radius * cos(phi2);
            float z2_2 = radius * sin(phi2) * sin(theta2);
            
            generateSquare(vertices,
                x1_1, y1_1, z1_1,  // p1
                x1_2, y1_2, z1_2,  // p2
                x2_1, y2_1, z2_1,  // p3
                x2_2, y2_2, z2_2); // p4
        }
    }
}

/**
 * Generates a cone centered at the origin with base on XZ plane
 * @param radius - Radius of the base
 * @param height - Height of the cone
 * @param slices - Number of slices around the base
 * @param stacks - Number of stacks along the height
 * @param vertices - Output list of vertices
 */
void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices) {
    float sliceStep = (2 * M_PI) / slices;
    float stackStep = height / stacks;

    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;

        // Base circle (y = 0) - triangle from center to edge (CW from below)
        float xb1 = radius * cos(theta1);
        float zb1 = radius * sin(theta1);
        float xb2 = radius * cos(theta2);
        float zb2 = radius * sin(theta2);


        addVertex(vertices, xb2, 0, zb2);
        addVertex(vertices, xb1, 0, zb1);
        addVertex(vertices, 0, 0, 0);

        // Stacks: each stack is a horizontal band between two rings
        for (int j = 0; j < stacks; j++) {
            float y_low = j * stackStep;
            float y_high = (j + 1) * stackStep;

            // Radius shrinks linearly from base to tip
            float r_low  = radius * (1.0f - (float)j / stacks);
            float r_high = radius * (1.0f - (float)(j + 1) / stacks);

            // 4 corners of the quad for this slice/stack
            float x1_low  = r_low  * cos(theta1);
            float z1_low  = r_low  * sin(theta1);
            float x2_low  = r_low  * cos(theta2);
            float z2_low  = r_low  * sin(theta2);

            float x1_high = r_high * cos(theta1);
            float z1_high = r_high * sin(theta1);
            float x2_high = r_high * cos(theta2);
            float z2_high = r_high * sin(theta2);

            if (j == stacks - 1) {
                // Top stack: single triangle to the apex (CW)
                addVertex(vertices, 0, height, 0);
                addVertex(vertices, x1_low, y_low, z1_low);
                addVertex(vertices, x2_low, y_low, z2_low);
            } else {
                // Normal stack: quad (2 triangles) (CW)
                generateSquare(vertices,
                    x1_low,  y_low,  z1_low,   // p1: bottom-left
                    x2_low,  y_low,  z2_low,   // p2: bottom-right
                    x1_high, y_high, z1_high,  // p3: top-left
                    x2_high, y_high, z2_high); // p4: top-right
            }
        }
    }
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
    else {
        cerr << "Unknown figure type: " << figure << endl;
        cerr << "Available shapes: sphere, box, cone, plane" << endl;
        return 1;
    }

    writeOutput(vertices, file);

    return 0;
}
