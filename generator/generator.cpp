#include "../include/generator_helpers.h"
#include "../include/figures.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <iostream>
#include <list>
#include <sstream>

using namespace std;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

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
void writeOutput(const list<string>& vertices, const string& file);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void addVertex(list<string>& vertices, float x, float y, float z) {
    stringstream ss;
    ss << x << " " << y << " " << z;
    vertices.push_back(ss.str());
}

void generateTriangle(list<string>& vertices,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float x3, float y3, float z3) {
    addVertex(vertices, x1, y1, z1);
    addVertex(vertices, x2, y2, z2);
    addVertex(vertices, x3, y3, z3);
}

void generateQuad(list<string>& vertices, 
                    float x1, float y1, float z1,
                    float x2, float y2, float z2,
                    float x3, float y3, float z3,
                    float x4, float y4, float z4) {
    generateTriangle(vertices, x1, y1, z1, x2, y2, z2, x4, y4, z4);
    generateTriangle(vertices, x1, y1, z1, x4, y4, z4, x3, y3, z3);
}

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

void writeOutput(const list<string>& vertices, const string& file) {
    string outputPath = "../../figures/" + file;
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << outputPath << endl;
        cerr << "Make sure the 'figures' directory exists!" << endl;
        return;
    }
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
// SCATTER — meta-generator
// ============================================================================

/**
 * Reads a .3d file and returns its vertex lines (skips the count header).
 */
vector<string> loadModel(const string& modelFile) {
    string modelPath = "../../figures/" + modelFile;
    ifstream f(modelPath);
    if (!f.is_open()) {
        cerr << "Error: Could not open model file " << modelPath << endl;
        return {};
    }
    vector<string> lines;
    string line;
    bool first = true;
    while (getline(f, line)) {
        if (first) { first = false; continue; } // skip vertex count
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

/**
 * Applies scale + rotation (Rx, Ry, Rz in radians) + translation to a vertex.
 * Order: scale → rotateX → rotateY → rotateZ → translate
 */
string transformVertex(const string& vertexLine,
                       float tx, float ty, float tz,
                       float rx, float ry, float rz,
                       float scale) {
    float x, y, z;
    istringstream ss(vertexLine);
    ss >> x >> y >> z;

    // Scale
    x *= scale; y *= scale; z *= scale;

    // Rotate X
    float y2 =  y * cos(rx) - z * sin(rx);
    float z2 =  y * sin(rx) + z * cos(rx);
    y = y2; z = z2;

    // Rotate Y
    float x2 =  x * cos(ry) + z * sin(ry);
    float z3 = -x * sin(ry) + z * cos(ry);
    x = x2; z = z3;

    // Rotate Z
    float x3 =  x * cos(rz) - y * sin(rz);
    float y3 =  x * sin(rz) + y * cos(rz);
    x = x3; y = y3;

    // Translate
    x += tx; y += ty; z += tz;

    ostringstream out;
    out << x << " " << y << " " << z;
    return out.str();
}

inline float rand01() { return (float)rand() / RAND_MAX; }

/**
 * Samples a random point inside a volume shell defined by shape + inner/outer params.
 *
 * Supported shapes:
 *   sphere  <r_min> <r_max>             — spherical shell
 *   torus   <R> <r_min> <r_max>         — toroidal shell  (flat: y is thin)
 *   plane   <width> <height>            — flat XZ plane
 *   cylinder <r_min> <r_max> <h_min> <h_max>  — cylindrical shell
 *   box     <inner_half> <outer_half>   — cubic shell (surface of hollow box)
 */
struct ScatterSample { float x, y, z; };

bool sampleVolume(const string& shape, const vector<float>& p,
                  ScatterSample& out) {
    if (shape == "sphere") {
        // p: r_min r_max
        float r = p[0] + rand01() * (p[1] - p[0]);
        float u = rand01(), v = rand01();
        float theta = 2.0f * M_PI * u;
        float phi   = acos(2.0f * v - 1.0f);
        out = { r * sin(phi) * cos(theta),
                r * sin(phi) * sin(theta),
                r * cos(phi) };

    } else if (shape == "torus") {
        // p: R r_min r_max
        float R = p[0];
        float r = p[1] + rand01() * (p[2] - p[1]);
        float u = rand01() * 2.0f * M_PI;
        float v = rand01() * 2.0f * M_PI;
        out = { (R + r * cos(v)) * cos(u),
                (R + r * cos(v)) * sin(u),
                 r * sin(v) };

    } else if (shape == "plane") {
        // p: width height
        out = { (rand01() - 0.5f) * p[0],
                0.0f,
                (rand01() - 0.5f) * p[1] };

    } else if (shape == "cylinder") {
        // p: r_min r_max h_min h_max
        float r = p[0] + rand01() * (p[1] - p[0]);
        float h = p[2] + rand01() * (p[3] - p[2]);
        float angle = rand01() * 2.0f * M_PI;
        out = { r * cos(angle), h, r * sin(angle) };

    } else if (shape == "box") {
        // p: inner_half outer_half
        // Pick a random point in the shell of the box by choosing a face
        // and a random position on that face, with depth between inner and outer.
        float inner = p[0], outer = p[1];
        int face = rand() % 6;
        float depth = inner + rand01() * (outer - inner);
        float u = (rand01() - 0.5f) * 2.0f * outer;
        float v = (rand01() - 0.5f) * 2.0f * outer;
        switch (face) {
            case 0: out = {  depth, u, v }; break;
            case 1: out = { -depth, u, v }; break;
            case 2: out = { u,  depth, v }; break;
            case 3: out = { u, -depth, v }; break;
            case 4: out = { u, v,  depth }; break;
            case 5: out = { u, v, -depth }; break;
        }
    } else {
        cerr << "scatter: unknown volume shape '" << shape << "'" << endl;
        cerr << "Supported: sphere, torus, plane, cylinder, box" << endl;
        return false;
    }
    return true;
}

/**
 * scatter <volume_shape> <volume_params...> <num> <model.3d> <scale_min> <scale_max> <output>
 *
 * Volume shapes and their params:
 *   sphere   <r_min> <r_max>
 *   torus    <R> <r_min> <r_max>
 *   plane    <width> <height>
 *   cylinder <r_min> <r_max> <h_min> <h_max>
 *   box      <inner_half> <outer_half>
 *
 * Examples:
 *   scatter sphere 450 490 1000 octahedron.3d 0.3 0.8 stars.3d
 *   scatter torus 110 8 15 200 asteroid.3d 0.5 2.0 belt_main.3d
 *   scatter plane 100 100 50 rock.3d 1.0 3.0 debris.3d
 */
int handleScatter(list<string>& arglist) {
    if (arglist.empty()) {
        cerr << "scatter: missing volume shape" << endl;
        return 1;
    }

    string shape = arglist.front(); arglist.pop_front();

    // Number of volume params per shape
    int numParams = 0;
    if      (shape == "sphere")   numParams = 2;
    else if (shape == "torus")    numParams = 3;
    else if (shape == "plane")    numParams = 2;
    else if (shape == "cylinder") numParams = 4;
    else if (shape == "box")      numParams = 2;
    else {
        cerr << "scatter: unknown volume shape '" << shape << "'" << endl;
        return 1;
    }

    if ((int)arglist.size() < numParams + 4) { // params + num + model + smin + smax
        cerr << "scatter: not enough arguments for shape '" << shape << "'" << endl;
        return 1;
    }

    vector<float> params;
    for (int i = 0; i < numParams; i++) {
        params.push_back(stof(arglist.front()));
        arglist.pop_front();
    }

    int   num       = stoi(arglist.front()); arglist.pop_front();
    string modelFile =      arglist.front();  arglist.pop_front();
    float scaleMin  = stof(arglist.front()); arglist.pop_front();
    float scaleMax  = stof(arglist.front()); arglist.pop_front();

    // Load the model to scatter
    vector<string> modelVerts = loadModel(modelFile);
    if (modelVerts.empty()) return 1;

    srand(42);
    list<string> vertices;

    for (int i = 0; i < num; i++) {
        ScatterSample pos;
        if (!sampleVolume(shape, params, pos)) return 1;

        float scale = scaleMin + rand01() * (scaleMax - scaleMin);
        float rx    = rand01() * 2.0f * M_PI;
        float ry    = rand01() * 2.0f * M_PI;
        float rz    = rand01() * 2.0f * M_PI;

        for (const string& v : modelVerts) {
            vertices.push_back(
                transformVertex(v, pos.x, pos.y, pos.z, rx, ry, rz, scale)
            );
        }
    }

    return 0; // caller will writeOutput
    // We return vertices via the passed-in list — see main below
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]){
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
        cerr << "  stars <shape> <num> <param1> <param2> <size1> <size2> <output_file>" << endl;
        cerr << "  scatter <volume_shape> <volume_params...> <num> <model.3d> <scale_min> <scale_max> <output_file>" << endl;
        cerr << "    volume_shape: sphere <r_min> <r_max>" << endl;
        cerr << "                  torus  <R> <r_min> <r_max>" << endl;
        cerr << "                  plane  <width> <height>" << endl;
        cerr << "                  cylinder <r_min> <r_max> <h_min> <h_max>" << endl;
        cerr << "                  box    <inner_half> <outer_half>" << endl;
        return 1;
    }

    list<string> arglist;
    string figure = argv[1];
    string file   = argv[argc - 1];

    for (int i = 2; i < argc - 1; i++)
        arglist.push_back(argv[i]);

    list<string> vertices;

    // ── primitive shapes (unchanged) ────────────────────────────────────────

    if (figure == "sphere") {
        if (arglist.size() != 3) { cerr << "Usage: sphere <radius> <slices> <stacks> <output_file>" << endl; return 1; }
        float radius = stof(arglist.front()); arglist.pop_front();
        int slices   = stoi(arglist.front()); arglist.pop_front();
        int stacks   = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("slices", slices, 1)    ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateSphere(radius, slices, stacks, vertices);

    } else if (figure == "box") {
        if (arglist.size() != 2) { cerr << "Usage: box <length> <divisions> <output_file>" << endl; return 1; }
        float length   = stof(arglist.front()); arglist.pop_front();
        int   divisions = stoi(arglist.front());
        if (!verifyMetric("length", length, 0.01) ||
            !verifyMetric("divisions", divisions, 1)) return 1;
        generateBox(length, divisions, vertices);

    } else if (figure == "cone") {
        if (arglist.size() != 4) { cerr << "Usage: cone <radius> <height> <slices> <stacks> <output_file>" << endl; return 1; }
        float radius = stof(arglist.front()); arglist.pop_front();
        float height = stof(arglist.front()); arglist.pop_front();
        int slices   = stoi(arglist.front()); arglist.pop_front();
        int stacks   = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("height", height, 0.01) ||
            !verifyMetric("slices", slices, 1)    ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateCone(radius, height, slices, stacks, vertices);

    } else if (figure == "plane") {
        if (arglist.size() != 2) { cerr << "Usage: plane <length> <divisions> <output_file>" << endl; return 1; }
        float length   = stof(arglist.front()); arglist.pop_front();
        int   divisions = stoi(arglist.front());
        if (!verifyMetric("length", length, 0.01) ||
            !verifyMetric("divisions", divisions, 1)) return 1;
        generatePlane(length, divisions, vertices);

    } else if (figure == "cylinder") {
        if (arglist.size() != 4) { cerr << "Usage: cylinder <radius> <height> <slices> <stacks> <output_file>" << endl; return 1; }
        float radius = stof(arglist.front()); arglist.pop_front();
        float height = stof(arglist.front()); arglist.pop_front();
        int slices   = stoi(arglist.front()); arglist.pop_front();
        int stacks   = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("height", height, 0.01) ||
            !verifyMetric("slices", slices, 1)    ||
            !verifyMetric("stacks", stacks, 1)) return 1;
        generateCylinder(radius, height, slices, stacks, vertices);

    } else if (figure == "icosphere") {
        if (arglist.size() != 2) { cerr << "Usage: icosphere <radius> <subdivisions> <output_file>" << endl; return 1; }
        float radius      = stof(arglist.front()); arglist.pop_front();
        int   subdivisions = stoi(arglist.front());
        if (!verifyMetric("radius", radius, 0.01) ||
            !verifyMetric("subdivisions", subdivisions, 0)) return 1;
        generateIcosphere(radius, subdivisions, vertices);

    } else if (figure == "torus") {
        if (arglist.size() != 4) { cerr << "Usage: torus <R> <r> <slices> <stacks> <output_file>" << endl; return 1; }
        float R  = stof(arglist.front()); arglist.pop_front();
        float r  = stof(arglist.front()); arglist.pop_front();
        int slices = stoi(arglist.front()); arglist.pop_front();
        int stacks = stoi(arglist.front());
        generateTorus(R, r, slices, stacks, vertices);

    } else if (figure == "ring") {
        if (arglist.size() != 3) { cerr << "Usage: ring <innerRadius> <outerRadius> <slices> <output_file>" << endl; return 1; }
        float innerR = stof(arglist.front()); arglist.pop_front();
        float outerR = stof(arglist.front()); arglist.pop_front();
        int   slices  = stoi(arglist.front());
        if (!verifyMetric("innerRadius", innerR, 0.0f) ||
            !verifyMetric("outerRadius", outerR, 0.01f) ||
            !verifyMetric("slices", slices, 3)) return 1;
        generateRing(innerR, outerR, slices, vertices);

    // ── stars (legacy, kept for compatibility) ───────────────────────────────
    } else if (figure == "octahedron") {
        if (arglist.size() != 1) { cerr << "Usage: octahedron <scale> <output_file>" << endl; return 1; }
        float scale = stof(arglist.front());
        if (!verifyMetric("scale", scale, 0.01)) return 1;
        generateOctahedron(vertices, 0.0f, 0.0f, 0.0f, scale);

    // ── scatter — meta-generator ─────────────────────────────────────────────
    } else if (figure == "scatter") {
        /*
         * scatter <volume_shape> <volume_params...> <num> <model.3d> <scale_min> <scale_max>
         *
         * volume_shape  params needed
         * ------------  --------------------------
         * sphere        r_min r_max
         * torus         R r_min r_max
         * plane         width height
         * cylinder      r_min r_max h_min h_max
         * box           inner_half outer_half
         *
         * Examples:
         *   scatter sphere 450 490 1000 octahedron.3d 0.3 0.8 stars.3d
         *   scatter torus 110 8 15 200 asteroid.3d 0.5 2.0 belt_main.3d
         *   scatter torus 290 12 25 500 asteroid.3d 0.3 3.0 belt_kuiper.3d
         */
        if (arglist.empty()) {
            cerr << "scatter: missing volume shape" << endl; return 1;
        }

        string shape = arglist.front(); arglist.pop_front();

        int numParams = 0;
        if      (shape == "sphere")   numParams = 2;
        else if (shape == "torus")    numParams = 3;
        else if (shape == "plane")    numParams = 2;
        else if (shape == "cylinder") numParams = 4;
        else if (shape == "box")      numParams = 2;
        else { cerr << "scatter: unknown volume shape '" << shape << "'" << endl; return 1; }

        if ((int)arglist.size() < numParams + 4) {
            cerr << "scatter: not enough arguments for shape '" << shape << "'" << endl;
            return 1;
        }

        vector<float> params;
        for (int i = 0; i < numParams; i++) {
            params.push_back(stof(arglist.front()));
            arglist.pop_front();
        }

        int    num       = stoi(arglist.front()); arglist.pop_front();
        string modelFile =      arglist.front();  arglist.pop_front();
        float  scaleMin  = stof(arglist.front()); arglist.pop_front();
        float  scaleMax  = stof(arglist.front()); arglist.pop_front();

        vector<string> modelVerts = loadModel(modelFile);
        if (modelVerts.empty()) return 1;

        srand(42);
        for (int i = 0; i < num; i++) {
            ScatterSample pos;
            if (!sampleVolume(shape, params, pos)) return 1;

            float scale = scaleMin + rand01() * (scaleMax - scaleMin);
            float rx    = rand01() * 2.0f * M_PI;
            float ry    = rand01() * 2.0f * M_PI;
            float rz    = rand01() * 2.0f * M_PI;

            for (const string& v : modelVerts)
                vertices.push_back(transformVertex(v, pos.x, pos.y, pos.z, rx, ry, rz, scale));
        }

    } else {
        cerr << "Unknown figure type: " << figure << endl;
        cerr << "Available shapes: sphere, box, cone, plane, cylinder, icosphere, torus, ring, stars, scatter" << endl;
        return 1;
    }

    writeOutput(vertices, file);
    return 0;
}