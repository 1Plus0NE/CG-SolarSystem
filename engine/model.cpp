#include "model.h"
#include "math_helpers.h"
#include "log_system.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <GL/glew.h>

using namespace std;

map<string, list<Vertex>> modelCache;

namespace {

// Planar projection UV: picks the two axes most perpendicular to the normal.
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

static Vertex makeVertex(const Vec3& p, const Vec3& n, const Vec2& uv) {
    return Vertex(p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1]);
}

static Vec3 parseVec3Line(const string& line) {
    istringstream iss(line);
    Vec3 p{};
    iss >> p[0] >> p[1] >> p[2];
    return p;
}

static bool parseObjFaceToken(const string& token,
                              int positionCount,
                              int texcoordCount,
                              int normalCount,
                              int& vIdx,
                              int& vtIdx,
                              int& vnIdx) {
    vIdx = vtIdx = vnIdx = -1;

    size_t first = token.find('/');
    if (first == string::npos) {
        vIdx = stoi(token);
    } else {
        size_t second = token.find('/', first + 1);
        string vPart = token.substr(0, first);
        string vtPart = (second == string::npos) ? token.substr(first + 1) : token.substr(first + 1, second - first - 1);
        string vnPart = (second == string::npos) ? string() : token.substr(second + 1);

        if (!vPart.empty()) vIdx = stoi(vPart);
        if (!vtPart.empty()) vtIdx = stoi(vtPart);
        if (!vnPart.empty()) vnIdx = stoi(vnPart);
    }

    auto resolveIndex = [](int idx, int count) -> int {
        if (idx > 0) return idx - 1;
        if (idx < 0) return count + idx;
        return -1;
    };

    vIdx = resolveIndex(vIdx, positionCount);
    vtIdx = resolveIndex(vtIdx, texcoordCount);
    vnIdx = resolveIndex(vnIdx, normalCount);
    return vIdx >= 0 && vIdx < positionCount;
}

static list<Vertex> loadLegacyModel(ifstream& file, const string& filename) {
    list<Vertex> vertices;
    string line;

    int expectedCount = -1;
    if (getline(file, line)) {
        istringstream iss(line);
        iss >> expectedCount;  // vertex count header — informational, not enforced
    }

    vector<Vec3> positions;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        Vec3 p{};
        if (iss >> p[0] >> p[1] >> p[2]) {
            positions.push_back(p);
        }
    }

    if (positions.size() % 3 != 0) {
        LOG_WARN("legacy model '" << filename << "' has incomplete triangle data.");
    }

    for (size_t i = 0; i + 2 < positions.size(); i += 3) {
        const Vec3& p0 = positions[i];
        const Vec3& p1 = positions[i + 1];
        const Vec3& p2 = positions[i + 2];
        Vec3 n = cross(subtract(p1, p0), subtract(p2, p0));
        normalize(n);
        auto uv = buildTriangleUVs(p0, p1, p2, n);
        vertices.push_back(makeVertex(p0, n, uv[0]));
        vertices.push_back(makeVertex(p1, n, uv[1]));
        vertices.push_back(makeVertex(p2, n, uv[2]));
    }

    (void)expectedCount;
    return vertices;
}

static list<Vertex> loadObjModel(ifstream& file, const string& filename) {
    list<Vertex> vertices;
    vector<Vec3> positions;
    vector<Vec2> texcoords;
    vector<Vec3> normals;
    vector<vector<string>> facesTokens;

    string line;
    // Collect all geometry data first; defer face triangulation to avoid
    // forward-reference issues with vn/vt defined after f lines.
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        string tag;
        iss >> tag;

        if (tag == "v") {
            Vec3 p{};
            if (iss >> p[0] >> p[1] >> p[2]) positions.push_back(p);
        } else if (tag == "vt") {
            Vec2 uv{0.0f, 0.0f};
            iss >> uv[0] >> uv[1];
            texcoords.push_back(uv);
        } else if (tag == "vn") {
            Vec3 n{};
            if (iss >> n[0] >> n[1] >> n[2]) {
                normalize(n);
                normals.push_back(n);
            }
        } else if (tag == "f") {
            vector<string> tokens;
            string token;
            while (iss >> token) tokens.push_back(token);
            if (tokens.size() >= 3) facesTokens.push_back(tokens);
        }
    }

    // When the file provides vn normals, use them directly (per-vertex vn overrides face normal).
    if (!normals.empty()) {
        for (const auto& tokens : facesTokens) {
            for (size_t i = 1; i + 1 < tokens.size(); ++i) {  // fan triangulation
                string a = tokens[0];
                string b = tokens[i];
                string c = tokens[i + 1];

                int vi[3], vti[3], vni[3];
                if (!parseObjFaceToken(a, (int)positions.size(), (int)texcoords.size(), (int)normals.size(), vi[0], vti[0], vni[0]) ||
                    !parseObjFaceToken(b, (int)positions.size(), (int)texcoords.size(), (int)normals.size(), vi[1], vti[1], vni[1]) ||
                    !parseObjFaceToken(c, (int)positions.size(), (int)texcoords.size(), (int)normals.size(), vi[2], vti[2], vni[2])) {
                    continue;
                }

                const Vec3& p0 = positions[vi[0]];
                const Vec3& p1 = positions[vi[1]];
                const Vec3& p2 = positions[vi[2]];

                Vec3 faceNormal = cross(subtract(p1, p0), subtract(p2, p0));
                normalize(faceNormal);

                auto triUV = buildTriangleUVs(p0, p1, p2, faceNormal);

                for (int k = 0; k < 3; ++k) {
                    const Vec3& p = positions[vi[k]];
                    Vec3 n = faceNormal;
                    if (vni[k] >= 0 && vni[k] < (int)normals.size()) n = normals[vni[k]];

                    Vec2 uv = triUV[k];
                    if (vti[k] >= 0 && vti[k] < (int)texcoords.size()) uv = texcoords[vti[k]];

                    vertices.push_back(makeVertex(p, n, uv));
                }
            }
        }
        return vertices;
    }

    // No vn normals: compute smooth normals by accumulating face normals per position,
    // then normalise. UV preference: explicit vt > planar projection.
    vector<Vec3> accumNormals(positions.size(), {0.0f, 0.0f, 0.0f});
    vector<Vec2> vertexUV(positions.size(), {0.0f, 0.0f});
    vector<char> haveUV(positions.size(), 0);

    for (const auto& tokens : facesTokens) {
        for (size_t i = 1; i + 1 < tokens.size(); ++i) {
            string a = tokens[0];
            string b = tokens[i];
            string c = tokens[i + 1];

            int vi[3], vti[3], vni[3];
            if (!parseObjFaceToken(a, (int)positions.size(), (int)texcoords.size(), 0, vi[0], vti[0], vni[0]) ||
                !parseObjFaceToken(b, (int)positions.size(), (int)texcoords.size(), 0, vi[1], vti[1], vni[1]) ||
                !parseObjFaceToken(c, (int)positions.size(), (int)texcoords.size(), 0, vi[2], vti[2], vni[2])) {
                continue;
            }

            const Vec3& p0 = positions[vi[0]];
            const Vec3& p1 = positions[vi[1]];
            const Vec3& p2 = positions[vi[2]];

            Vec3 faceNormal = cross(subtract(p1, p0), subtract(p2, p0));
            normalize(faceNormal);

            auto triUV = buildTriangleUVs(p0, p1, p2, faceNormal);

            for (int k = 0; k < 3; ++k) {
                accumNormals[vi[k]][0] += faceNormal[0];
                accumNormals[vi[k]][1] += faceNormal[1];
                accumNormals[vi[k]][2] += faceNormal[2];

                if (!haveUV[vi[k]]) {
                    if (vti[k] >= 0 && vti[k] < (int)texcoords.size()) {
                        vertexUV[vi[k]] = texcoords[vti[k]];
                        haveUV[vi[k]] = 1;
                    } else {
                        vertexUV[vi[k]] = triUV[k];
                        haveUV[vi[k]] = 1;
                    }
                }
            }
        }
    }

    // Second pass: emit vertices with normalised smooth normals.
    for (const auto& tokens : facesTokens) {
        for (size_t i = 1; i + 1 < tokens.size(); ++i) {
            string a = tokens[0];
            string b = tokens[i];
            string c = tokens[i + 1];

            int vi[3], vti[3], vni[3];
            if (!parseObjFaceToken(a, (int)positions.size(), (int)texcoords.size(), 0, vi[0], vti[0], vni[0]) ||
                !parseObjFaceToken(b, (int)positions.size(), (int)texcoords.size(), 0, vi[1], vti[1], vni[1]) ||
                !parseObjFaceToken(c, (int)positions.size(), (int)texcoords.size(), 0, vi[2], vti[2], vni[2])) {
                continue;
            }

            for (int k = 0; k < 3; ++k) {
                Vec3 n = accumNormals[vi[k]];
                normalize(n);
                Vec2 uv = vertexUV[vi[k]];
                const Vec3& p = positions[vi[k]];
                vertices.push_back(makeVertex(p, n, uv));
            }
        }
    }

    (void)filename;
    return vertices;
}

} // namespace

// Interleaved VBO layout: [x y z nx ny nz u v] × vertexCount.
// Allocates VAO+VBO on first call; re-uploads on subsequent calls (DYNAMIC models).
// Returns false and leaves m.gpuReady=false if the upload fails.
bool uploadModelToGPU(Model& m) {
    if (m.vertices.empty()) {
        LOG_WARN("uploadModelToGPU: '" << m.file << "' has no vertices, skipping.");
        return false;
    }

    vector<float> buf;
    buf.reserve(m.vertices.size() * 8);
    for (const auto& v : m.vertices) {
        buf.push_back(v.x);
        buf.push_back(v.y);
        buf.push_back(v.z);
        buf.push_back(v.nx);
        buf.push_back(v.ny);
        buf.push_back(v.nz);
        buf.push_back(v.u);
        buf.push_back(v.v);
    }
    m.vertexCount = (int)m.vertices.size();

    GLenum usage = (m.renderMode == STATIC)
                 ? GL_STATIC_DRAW
                 : GL_DYNAMIC_DRAW;

    if (!m.gpuReady) {
        glGenVertexArrays(1, &m.vaoId);
        glGenBuffers(1, &m.vboId);
        if (!m.vaoId || !m.vboId) {
            LOG_ERROR("uploadModelToGPU: VAO/VBO allocation failed for '" << m.file << "'.");
            if (m.vaoId) { glDeleteVertexArrays(1, &m.vaoId); m.vaoId = 0; }
            if (m.vboId) { glDeleteBuffers(1, &m.vboId);      m.vboId = 0; }
            return false;
        }
    }

    glBindVertexArray(m.vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m.vboId);
    glBufferData(GL_ARRAY_BUFFER,
                 buf.size() * sizeof(float),
                 buf.data(), usage);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOG_ERROR("uploadModelToGPU: glBufferData error 0x" << hex << err
                  << " for '" << m.file << "'.");
        glBindVertexArray(0);
        return false;
    }

    const GLsizei stride = 8 * sizeof(float);
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(6 * sizeof(float)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindVertexArray(0);
    m.isDirty  = false;
    m.gpuReady = true;
    return true;
}

void freeModelGPU(Model& m) {
    if (!m.gpuReady) return;
    glDeleteVertexArrays(1, &m.vaoId);
    glDeleteBuffers(1, &m.vboId);
    m.gpuReady = false;
}

// Format detection: OBJ keywords (v/vt/vn/f/o/g…) → OBJ path;
// numeric first token → legacy .3d path; ambiguous → OBJ path.
list<Vertex> loadModelFile(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR("Could not open model file " << filename);
        return {};
    }

    string firstMeaningfulLine;
    streampos startPos = file.tellg();
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        firstMeaningfulLine = line;
        break;
    }

    file.clear();
    file.seekg(startPos);

    if (!firstMeaningfulLine.empty()) {
        istringstream iss(firstMeaningfulLine);
        string tag;
        iss >> tag;
        if (tag == "v" || tag == "vt" || tag == "vn" || tag == "f" ||
            tag == "o" || tag == "g" || tag == "s" || tag == "usemtl" || tag == "mtllib") {
            return loadObjModel(file, filename);
        }

        bool legacyLooksNumeric = !tag.empty() && (isdigit((unsigned char)tag[0]) || tag[0] == '-' || tag[0] == '+');
        if (legacyLooksNumeric) {
            return loadLegacyModel(file, filename);
        }
    }

    return loadObjModel(file, filename);
}

// Returns cached vertices; on miss, tries ../../figures/<filename> and then
// flips the extension between .3d and .obj if the primary path is missing.
list<Vertex> getModelVertices(const string& filename) {
    if (modelCache.find(filename) == modelCache.end()) {
        string basePath = "../../figures/" + filename;
        ifstream test(basePath.c_str());
        if (!test.is_open()) {
            string altPath = basePath;
            size_t dot = altPath.find_last_of('.');
            if (dot != string::npos) {
                string ext = altPath.substr(dot + 1);
                if (ext == "3d") {
                    altPath = altPath.substr(0, dot) + ".obj";
                } else if (ext == "obj") {
                    altPath = altPath.substr(0, dot) + ".3d";
                }
            }
            modelCache[filename] = loadModelFile(altPath.c_str());
        } else {
            modelCache[filename] = loadModelFile(basePath.c_str());
        }
    }
    return modelCache[filename];
}

void clearModelCache() {
    modelCache.clear();
}
