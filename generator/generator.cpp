#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <cmath>
#include <vector>

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

// Shape generators
void generateBox(float length, int divisions, list<string>& vertices);
void generatePlane(float length, int divisions, list<string>& vertices);
void generateSphere(float radius, int slices, int stacks, list<string>& vertices);
void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices);
void generateCylinder(float radius, float height, int slices, list<string>& vertices);
void generateIcosphere(float radius, int subdivisions, list<string>& vertices);

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
            generateQuad(vertices,
                x0, y0, half,   // p1: bottom-left
                x1, y0, half,   // p2: bottom-right
                x0, y1, half,   // p3: top-left
                x1, y1, half);  // p4: top-right
            
            // Back face (z = -half) - looking from -Z towards origin
            // CCW when viewed from outside: bottom-right, bottom-left, top-right, top-left
            generateQuad(vertices,
                x1, y0, -half,  // p1: bottom-right
                x0, y0, -half,  // p2: bottom-left
                x1, y1, -half,  // p3: top-right
                x0, y1, -half); // p4: top-left
            
            // Right face (x = +half) - looking from +X towards origin
            // CCW when viewed from outside
            generateQuad(vertices,
                half, y0, x1,   // p1
                half, y0, x0,   // p2
                half, y1, x1,   // p3
                half, y1, x0);  // p4
            
            // Left face (x = -half) - looking from -X towards origin
            // CCW when viewed from outside
            generateQuad(vertices,
                -half, y0, x0,  // p1
                -half, y0, x1,  // p2
                -half, y1, x0,  // p3
                -half, y1, x1); // p4
            
            // Top face (y = +half) - looking from +Y downwards
            // CCW when viewed from outside
            generateQuad(vertices,
                x0, half, y1,   // p1
                x1, half, y1,   // p2
                x0, half, y0,   // p3
                x1, half, y0);  // p4
            
            // Bottom face (y = -half) - looking from -Y upwards
            // CCW when viewed from outside
            generateQuad(vertices,
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
            generateQuad(vertices,
                x0, 0.0f, z0,  // p1: bottom-left
                x1, 0.0f, z0,  // p2: bottom-right
                x0, 0.0f, z1,  // p3: top-left
                x1, 0.0f, z1); // p4: top-right

            // Bottom face - CCW when viewed from below (-Y)
            generateQuad(vertices,
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
            
            if (i == 0) {
                // Top pole: phi1 = 0, all upper points collapse to (0, radius, 0)
                // Only need a single triangle from the pole to the lower ring
                float x2_1 = radius * sin(phi2) * cos(theta1);
                float y2_1 = radius * cos(phi2);
                float z2_1 = radius * sin(phi2) * sin(theta1);
                
                float x2_2 = radius * sin(phi2) * cos(theta2);
                float y2_2 = radius * cos(phi2);
                float z2_2 = radius * sin(phi2) * sin(theta2);
                
                // CCW when viewed from outside: pole -> bottom-right -> bottom-left
                generateTriangle(vertices,
                    0, radius, 0,
                    x2_2, y2_2, z2_2,
                    x2_1, y2_1, z2_1);

            } else if (i == stacks - 1) {
                // Bottom pole: phi2 = PI, all lower points collapse to (0, -radius, 0)
                // Only need a single triangle from the upper ring to the pole
                float x1_1 = radius * sin(phi1) * cos(theta1);
                float y1_1 = radius * cos(phi1);
                float z1_1 = radius * sin(phi1) * sin(theta1);
                
                float x1_2 = radius * sin(phi1) * cos(theta2);
                float y1_2 = radius * cos(phi1);
                float z1_2 = radius * sin(phi1) * sin(theta2);
                
                // CCW when viewed from outside: top-left -> top-right -> pole
                generateTriangle(vertices,
                    x1_1, y1_1, z1_1,
                    x1_2, y1_2, z1_2,
                    0, -radius, 0);

            } else {
                // Normal stack: full quad between two rings
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
                
                generateQuad(vertices,
                    x1_1, y1_1, z1_1,  // p1
                    x1_2, y1_2, z1_2,  // p2
                    x2_1, y2_1, z2_1,  // p3
                    x2_2, y2_2, z2_2); // p4
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
    float sliceStep = (2 * M_PI) / slices;
    float stackStep = height / stacks;

    // Base circle (y = 0)
    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;
        float xb1 = radius * cos(theta1);
        float zb1 = radius * sin(theta1);
        float xb2 = radius * cos(theta2);
        float zb2 = radius * sin(theta2);
        generateTriangle(vertices, xb1, 0, zb1, xb2, 0, zb2, 0, 0, 0);
    }

    // Stacks: iterate over height levels first
    for (int j = 0; j < stacks; j++) {
        float y_low = j * stackStep;
        float y_high = (j + 1) * stackStep;

        // Calculate radii once per stack (linear regression)
        float r_low  = radius * (1.0f - (float)j / stacks);
        float r_high = radius * (1.0f - (float)(j + 1) / stacks);

        // Then iterate over slices
        for (int i = 0; i < slices; i++) {
            float theta1 = i * sliceStep;
            float theta2 = (i + 1) * sliceStep;

            float x1_low  = r_low  * cos(theta1);
            float z1_low  = r_low  * sin(theta1);
            float x2_low  = r_low  * cos(theta2);
            float z2_low  = r_low  * sin(theta2);

            float x1_high = r_high * cos(theta1);
            float z1_high = r_high * sin(theta1);
            float x2_high = r_high * cos(theta2);
            float z2_high = r_high * sin(theta2);

            if (j == stacks - 1) {
                   generateTriangle(vertices, 0, height, 0, x2_low, y_low, z2_low, x1_low, y_low, z1_low);
            } else {
                generateQuad(vertices,
                    x1_low,  y_low,  z1_low,      // p1
                    x1_high, y_high, z1_high,     // p3 (trocado)
                    x2_low,  y_low,  z2_low,      // p2 (trocado)
                    x2_high, y_high, z2_high);    // p4
            }
        }
    }
}

/**
 * Generates a cylinder centered at the origin with base on XZ plane
 * @param radius - Radius of the cylinder
 * @param height - Height of the cylinder
 * @param slices - Number of slices around the base
 * @param vertices - Output list of vertices
 */
void generateCylinder(float radius, float height, int slices, list<string>& vertices) {
    float sliceStep = (2 * M_PI) / slices;

    for (int i = 0; i < slices; i++) {
        float theta1 = i * sliceStep;
        float theta2 = (i + 1) * sliceStep;

        float x1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);
        float x2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        // Bottom base (y = 0) - CCW looking from below (outward = downward)
        generateTriangle(vertices, x1, 0, z1, x2, 0, z2, 0, 0, 0);

        // Top base (y = height) - CCW looking from above (outward = upward)  
        generateTriangle(vertices, x1, height, z1, 0, height, 0, x2, height, z2);

        // Side body - quads
        generateQuad(vertices,
            x2, 0,      z2,  // bottom left
            x1, 0,      z1,  // bottom right
            x2, height, z2,  // top left
            x1, height, z1   // top right
        );

    }
}
// Lightweight 3D Point struct for icosphere generation
struct Point3D {
    float x, y, z;
    Point3D(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    
    // Normalize point to be on sphere of given radius
    Point3D normalize(float radius) const {
        float length = sqrt(x*x + y*y + z*z);
        return Point3D(x * radius / length, y * radius / length, z * radius / length);
    }
};

/**
 * Generates a Torus centered at the origin
 * @param ringRadius - Distance from the origin to the center of the tube (R)
 * @param pipeRadius - Radius of the tube's cross-section (r)
 * @param slices - Number of divisions around the main ring
 * @param stacks - Number of divisions around the tube's section
 * @param vertices - Output list of vertices
 */
void generateTorus(float ringRadius, float pipeRadius, int slices, int stacks, list<string>& vertices) {
    float sliceStep = 2.0f * M_PI / slices;
    float stackStep = 2.0f * M_PI / stacks;

    for (int i = 0; i < slices; i++) {
        float u1 = i * sliceStep;
        float u2 = (i + 1) * sliceStep;

        for (int j = 0; j < stacks; j++) {
            float v1 = j * stackStep;
            float v2 = (j + 1) * stackStep;

            // Lambda function to calculate parametric position
            auto getPoint = [&](float u, float v) {
                // The horizontal distance from the Y axis to the point
                // Projecting the pipe radius onto the XZ plane using cos(v)
                float horizontalDist = ringRadius + pipeRadius * cos(v);
                
                // x = (R + r*cos(v)) * cos(u)
                // y = r * sin(v)
                // z = (R + r*cos(v)) * sin(u)
                float x = horizontalDist * cos(u);
                float y = pipeRadius * sin(v);
                float z = horizontalDist * sin(u);
                
                return Point3D(x, y, z);
            };

            // Calculate the 4 corners of the current grid cell
            Point3D p1 = getPoint(u1, v1); // current slice, current stack
            Point3D p2 = getPoint(u2, v1); // next slice, same stack
            Point3D p3 = getPoint(u1, v2); // same slice, next stack
            Point3D p4 = getPoint(u2, v2); // next slice, next stack

            // The order of points in generateQuad (p1, p2, p3, p4) 
            // handles the split into two CCW (Counter-Clockwise) triangles:
            // Triangle 1: p1 -> p2 -> p4
            // Triangle 2: p1 -> p4 -> p3
            generateQuad(vertices,
                p1.x, p1.y, p1.z,
                p2.x, p2.y, p2.z,
                p3.x, p3.y, p3.z,
                p4.x, p4.y, p4.z);
        }
    }
}

/**
 * Generates an icosphere (subdivided icosahedron) centered at origin
 * @param radius - Radius of the sphere
 * @param subdivisions - Number of times to subdivide the faces
 * @param vertices - Output list of vertices
 */
void generateIcosphere(float radius, int subdivisions, list<string>& vertices) {
    // Golden ratio
    const float t = (1.0f + sqrt(5.0f)) / 2.0f;

    // 12 vertices of an icosahedron
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

    // 20 faces of an icosahedron (indices of vertices)
    struct TriangleIndex { int v1, v2, v3; };
    vector<TriangleIndex> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
        
    };

    // Store current working set of vertices
    vector<Point3D> currentVertices = baseVertices;

    // Helper functions for subdivision
    // Cache for middle points to avoid duplicating vertices (simplified without cache here for straightforward generation)
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
            // Calculate 3 midpoints
            int a = getMiddlePoint(face.v1, face.v2);
            int b = getMiddlePoint(face.v2, face.v3);
            int c = getMiddlePoint(face.v3, face.v1);

            // Create 4 new triangles
            nextFaces.push_back({face.v1, a, c});
            nextFaces.push_back({face.v2, b, a});
            nextFaces.push_back({face.v3, c, b});
            nextFaces.push_back({a, b, c});
        }
        faces = nextFaces;
    }

    // Convert to generator string vertex format
    for(const auto& face : faces) {
        // Ensure CCW outer facing
        generateTriangle(vertices,
            currentVertices[face.v1].x, currentVertices[face.v1].y, currentVertices[face.v1].z,
            currentVertices[face.v2].x, currentVertices[face.v2].y, currentVertices[face.v2].z,
            currentVertices[face.v3].x, currentVertices[face.v3].y, currentVertices[face.v3].z
        );
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
        cerr << "  cylinder <radius> <height> <slices> <output_file>" << endl;
        cerr << "  icosphere <radius> <subdivisions> <output_file>" << endl;
        cerr << "  torus <R> <r> <slices> <stacks> <output_file>" << endl;
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
        if (arglist.size() != 3) {
            cerr << "Usage: cylinder <radius> <height> <slices> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        float height = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("height", height, 0.01) ||
            !verifyMetric("slices", slices, 1)) return 1;
        generateCylinder(radius, height, slices, vertices);
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
    else {
        cerr << "Unknown figure type: " << figure << endl;
        cerr << "Available shapes: sphere, box, cone, plane, cylinder, icosphere, torus" << endl;
        return 1;
    }

    writeOutput(vertices, file);

    return 0;
}



