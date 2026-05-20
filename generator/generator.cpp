#include "../include/generator_helpers.h"
#include "../include/figures.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <iostream>
#include <list>
#include <sstream>
#include <array>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <unordered_map>

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

namespace {

using Vec3 = array<float, 3>;
using Vec2 = array<float, 2>;

static Vec3 parseVec3(const string& line) {
    istringstream iss(line);
    Vec3 p{};
    iss >> p[0] >> p[1] >> p[2];
    return p;
}

static Vec3 subtractVec(const Vec3& a, const Vec3& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

static Vec3 crossVec(const Vec3& a, const Vec3& b) {
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

static void normalizeVec(Vec3& v) {
    float len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 1e-6f) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    } else {
        v = {0.0f, 0.0f, 1.0f};
    }
}

static Vec2 projectTexCoord(const Vec3& p, const Vec3& n) {
    float ax = fabs(n[0]);
    float ay = fabs(n[1]);
    float az = fabs(n[2]);

    if (ax >= ay && ax >= az) return {p[1], p[2]};
    if (ay >= ax && ay >= az) return {p[0], p[2]};
    return {p[0], p[1]};
}

static array<Vec2, 3> buildTriangleUVs(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& n) {
    array<Vec2, 3> projected = {
        projectTexCoord(p0, n),
        projectTexCoord(p1, n),
        projectTexCoord(p2, n)
    };

    float minU = projected[0][0], maxU = projected[0][0];
    float minV = projected[0][1], maxV = projected[0][1];
    for (int i = 1; i < 3; ++i) {
        minU = min(minU, projected[i][0]);
        maxU = max(maxU, projected[i][0]);
        minV = min(minV, projected[i][1]);
        maxV = max(maxV, projected[i][1]);
    }

    float du = maxU - minU;
    float dv = maxV - minV;
    array<Vec2, 3> uv = {};
    for (int i = 0; i < 3; ++i) {
        uv[i][0] = (du > 1e-6f) ? ((projected[i][0] - minU) / du) : 0.0f;
        uv[i][1] = (dv > 1e-6f) ? ((projected[i][1] - minV) / dv) : 0.0f;
    }
    return uv;
}

static void writeTriangleObj(ofstream& outFile, const Vec3& p0, const Vec3& p1, const Vec3& p2, int& indexBase) {
    Vec3 normal = crossVec(subtractVec(p1, p0), subtractVec(p2, p0));
    normalizeVec(normal);
    auto uvs = buildTriangleUVs(p0, p1, p2, normal);

    const Vec3 points[3] = {p0, p1, p2};
    for (int i = 0; i < 3; ++i) {
        outFile << "v " << fixed << setprecision(6)
                << points[i][0] << ' ' << points[i][1] << ' ' << points[i][2] << '\n';
        outFile << "vt " << fixed << setprecision(6)
                << uvs[i][0] << ' ' << uvs[i][1] << '\n';
        outFile << "vn " << fixed << setprecision(6)
                << normal[0] << ' ' << normal[1] << ' ' << normal[2] << '\n';
    }

    outFile << "f "
            << indexBase + 1 << '/' << indexBase + 1 << '/' << indexBase + 1 << ' '
            << indexBase + 2 << '/' << indexBase + 2 << '/' << indexBase + 2 << ' '
            << indexBase + 3 << '/' << indexBase + 3 << '/' << indexBase + 3 << '\n';
    indexBase += 3;
}

static vector<string> extractVertexLines(const string& modelPath) {
    ifstream f(modelPath);
    if (!f.is_open()) return {};

    vector<string> lines;
    string line;
    bool first = true;
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        string tag;
        iss >> tag;

        if (tag == "v") {
            string x, y, z;
            if (iss >> x >> y >> z) {
                lines.push_back(x + " " + y + " " + z);
            }
            continue;
        }

        if (first) {
            first = false;
            bool looksNumeric = !tag.empty() && (isdigit((unsigned char)tag[0]) || tag[0] == '-' || tag[0] == '+');
            if (looksNumeric) {
                continue; // legacy header
            }
        }

        if (!first) {
            string rest;
            getline(iss, rest);
        }
    }

    if (lines.empty()) {
        // legacy fallback: skip count header and keep raw coordinate lines
        f.clear();
        f.seekg(0);
        first = true;
        while (getline(f, line)) {
            if (first) { first = false; continue; }
            if (!line.empty()) lines.push_back(line);
        }
    }

    return lines;
}

} // namespace

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
    vector<Vec3> points;
    points.reserve(vertices.size());
    for (const auto& vertex : vertices) points.push_back(parseVec3(vertex));

    outFile << "# Generated by CG-SolarSystem\n";

    // Build unique vertex list by position (string key of coords), accumulate face normals
    unordered_map<string, int> posToIndex;
    vector<Vec3> uniquePos;
    vector<Vec3> accumNormals;
    vector<Vec2> accumUV;
    vector<int> uvCount;

    auto makeKey = [](const Vec3& p) {
        std::ostringstream ss;
        ss << fixed << setprecision(6) << p[0] << "," << p[1] << "," << p[2];
        return ss.str();
    };

    // First pass: accumulate normals and UVs per unique position
    for (size_t i = 0; i + 2 < points.size(); i += 3) {
        const Vec3& p0 = points[i];
        const Vec3& p1 = points[i + 1];
        const Vec3& p2 = points[i + 2];
        Vec3 normal = crossVec(subtractVec(p1, p0), subtractVec(p2, p0));
        normalizeVec(normal);
        auto triUV = buildTriangleUVs(p0, p1, p2, normal);

        const Vec3 triPoints[3] = {p0, p1, p2};
        for (int k = 0; k < 3; ++k) {
            string key = makeKey(triPoints[k]);
            int idx;
            auto it = posToIndex.find(key);
            if (it == posToIndex.end()) {
                idx = (int)uniquePos.size();
                posToIndex.emplace(key, idx);
                uniquePos.push_back(triPoints[k]);
                accumNormals.push_back({0.0f, 0.0f, 0.0f});
                accumUV.push_back({0.0f, 0.0f});
                uvCount.push_back(0);
            } else {
                idx = it->second;
            }

            accumNormals[idx][0] += normal[0];
            accumNormals[idx][1] += normal[1];
            accumNormals[idx][2] += normal[2];

            accumUV[idx][0] += triUV[k][0];
            accumUV[idx][1] += triUV[k][1];
            uvCount[idx]++;
        }
    }

    // Normalize accumulated normals and average UVs
    vector<Vec3> finalNormals(uniquePos.size());
    vector<Vec2> finalUVs(uniquePos.size());
    for (size_t i = 0; i < uniquePos.size(); ++i) {
        Vec3 n = accumNormals[i];
        normalizeVec(n);
        finalNormals[i] = n;
        if (uvCount[i] > 0) {
            finalUVs[i][0] = accumUV[i][0] / uvCount[i];
            finalUVs[i][1] = accumUV[i][1] / uvCount[i];
        } else {
            finalUVs[i] = {0.0f, 0.0f};
        }
    }

    // Write unique vertex attributes
    for (size_t i = 0; i < uniquePos.size(); ++i) {
        outFile << "v " << fixed << setprecision(6)
                << uniquePos[i][0] << ' ' << uniquePos[i][1] << ' ' << uniquePos[i][2] << '\n';
    }
    for (size_t i = 0; i < finalUVs.size(); ++i) {
        outFile << "vt " << fixed << setprecision(6)
                << finalUVs[i][0] << ' ' << finalUVs[i][1] << '\n';
    }
    for (size_t i = 0; i < finalNormals.size(); ++i) {
        outFile << "vn " << fixed << setprecision(6)
                << finalNormals[i][0] << ' ' << finalNormals[i][1] << ' ' << finalNormals[i][2] << '\n';
    }

    // Write faces referencing unique indices
    for (size_t i = 0; i + 2 < points.size(); i += 3) {
        const Vec3& p0 = points[i];
        const Vec3& p1 = points[i + 1];
        const Vec3& p2 = points[i + 2];
        string k0 = makeKey(p0);
        string k1 = makeKey(p1);
        string k2 = makeKey(p2);
        int idx0 = posToIndex[k0] + 1;
        int idx1 = posToIndex[k1] + 1;
        int idx2 = posToIndex[k2] + 1;
        outFile << "f "
                << idx0 << '/' << idx0 << '/' << idx0 << ' '
                << idx1 << '/' << idx1 << '/' << idx1 << ' '
                << idx2 << '/' << idx2 << '/' << idx2 << '\n';
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
 * Reads a model file and returns its vertex lines.
 * Supports legacy .3d and OBJ files.
 */
vector<string> loadModel(const string& modelFile) {
    string modelPath = "../../figures/" + modelFile;
    vector<string> lines = extractVertexLines(modelPath);
    if (lines.empty()) {
        string altPath = modelPath;
        size_t dot = altPath.find_last_of('.');
        if (dot != string::npos) {
            string ext = altPath.substr(dot + 1);
            if (ext == "3d") {
                altPath = altPath.substr(0, dot) + ".obj";
            } else if (ext == "obj") {
                altPath = altPath.substr(0, dot) + ".3d";
            }
        }
        if (altPath != modelPath) {
            lines = extractVertexLines(altPath);
            if (!lines.empty()) return lines;
        }
        cerr << "Error: Could not open model file " << modelPath << endl;
        return {};
    }
    return lines;
}

/**
 * scatter <volume_shape> <volume_params...> <num> <model.obj> <scale_min> <scale_max> <output>
 *
 * Volume shapes and their params:
 *   sphere   <r_min> <r_max>
 *   torus    <R> <r_min> <r_max>
 *   plane    <width> <height>
 *   cylinder <r_min> <r_max> <h_min> <h_max>
 *   box      <inner_half> <outer_half>
 *
 * Examples:
 *   scatter sphere 450 490 1000 octahedron.obj 0.3 0.8 stars.obj
 *   scatter torus 110 8 15 200 asteroid.obj 0.5 2.0 belt_main.obj
 *   scatter plane 100 100 50 rock.obj 1.0 3.0 debris.obj
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
        cerr << "  bezier <patchfile> <tessellation> <output_file>" << endl;
        cerr << "  scatter <volume_shape> <volume_params...> <num> <model.obj> <scale_min> <scale_max> <output_file>" << endl;
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

    } else if (figure == "bezier") {
        if (arglist.size() != 2) {
            cerr << "Usage: bezier <patchfile> <tessellation> <output>" << endl;
            return 1;
        }
        string patchFile = arglist.front(); arglist.pop_front();
        int tess = stoi(arglist.front());
        if (!verifyMetric("tessellation", tess, 1)) return 1;
        generateBezier(patchFile, tess, vertices);

    // ── scatter — meta-generator ─────────────────────────────────────────────
    } else if (figure == "scatter") {
        /*
         * scatter <volume_shape> <volume_params...> <num> <model.obj> <scale_min> <scale_max>
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
         *   scatter sphere 450 490 1000 octahedron.obj 0.3 0.8 stars.obj
         *   scatter torus 110 8 15 200 asteroid.obj 0.5 2.0 belt_main.obj
         *   scatter torus 290 12 25 500 asteroid.obj 0.3 3.0 belt_kuiper.obj
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