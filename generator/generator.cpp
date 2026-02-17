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

            // Generate plane on XZ plane (y = 0)
            // CCW when viewed from above (+Y)
            generateSquare(vertices,
                x0, 0.0f, z0,  // p1: bottom-left
                x1, 0.0f, z0,  // p2: bottom-right
                x0, 0.0f, z1,  // p3: top-left
                x1, 0.0f, z1); // p4: top-right
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
    float stackStep = M_PI / stacks;        // π / stacks (latitude step)
    float sliceStep = 2.0f * M_PI / slices; // 2π / slices (longitude step)
    
    // Generate sphere vertices stack by stack (from top to bottom)
    for (int i = 0; i < stacks; i++) {
        // Current and next stack angles (from north pole π/2 to south pole -π/2)
        float stackAngle = M_PI / 2 - i * stackStep;           // Current latitude
        float nextStackAngle = M_PI / 2 - (i + 1) * stackStep; // Next latitude
        
        // Calculate y coordinates and radii for current and next stack
        float y1 = radius * sin(stackAngle);
        float y2 = radius * sin(nextStackAngle);
        float r1 = radius * cos(stackAngle);     // Radius at current stack
        float r2 = radius * cos(nextStackAngle); // Radius at next stack
        
        for (int j = 0; j < slices; j++) {
            // Current and next slice angles (longitude)
            float sliceAngle = j * sliceStep;           // Current longitude
            float nextSliceAngle = (j + 1) * sliceStep; // Next longitude
            
            // Calculate the 4 vertices of the quad (which will be split into 2 triangles)
            // Stack i, slice j (top-left)
            float x1 = r1 * cos(sliceAngle);
            float z1 = r1 * sin(sliceAngle);
            
            // Stack i, slice j+1 (top-right)
            float x2 = r1 * cos(nextSliceAngle);
            float z2 = r1 * sin(nextSliceAngle);
            
            // Stack i+1, slice j (bottom-left)
            float x3 = r2 * cos(sliceAngle);
            float z3 = r2 * sin(sliceAngle);
            
            // Stack i+1, slice j+1 (bottom-right)
            float x4 = r2 * cos(nextSliceAngle);
            float z4 = r2 * sin(nextSliceAngle);
            
            // Generate 2 triangles in CCW order when viewed from outside
            // For a sphere, normals point outward, so we need CCW when viewed from outside
            
            // Special case: top cap (avoid degenerate triangles at north pole)
            if (i == 0) {
                // Only generate bottom triangle (x1,y1,z1 is at the pole, same point)
                addVertex(vertices, x1, y1, z1);  // Top vertex (pole)
                addVertex(vertices, x4, y2, z4);  // Bottom-right
                addVertex(vertices, x3, y2, z3);  // Bottom-left
            }
            // Special case: bottom cap (avoid degenerate triangles at south pole)
            else if (i == stacks - 1) {
                // Only generate top triangle (x3,y2,z3 is at the pole, same point)
                addVertex(vertices, x1, y1, z1);  // Top-left
                addVertex(vertices, x3, y2, z3);  // Bottom vertex (pole)
                addVertex(vertices, x2, y1, z2);  // Top-right
            }
            // Normal case: generate 2 triangles for the quad
            else {
                // Triangle 1: top-left, bottom-left, bottom-right (CCW from outside)
                addVertex(vertices, x1, y1, z1);  // Top-left
                addVertex(vertices, x3, y2, z3);  // Bottom-left
                addVertex(vertices, x4, y2, z4);  // Bottom-right
                
                // Triangle 2: top-left, bottom-right, top-right (CCW from outside)
                addVertex(vertices, x1, y1, z1);  // Top-left
                addVertex(vertices, x4, y2, z4);  // Bottom-right
                addVertex(vertices, x2, y1, z2);  // Top-right
            }
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
    // TODO: Implement cone generation
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
