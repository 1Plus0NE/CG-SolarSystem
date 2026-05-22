#include "rendering.h"
#include "model.h"
#include "config.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <cstdlib>
#include <cctype>
#include <string>
#include <GL/glew.h>
#include "texture.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

static string directoryOf(const string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos) return string();
    return path.substr(0, pos + 1);
}

static string resolveTexturePath(const string& textureFile) {
    if (textureFile.empty()) return textureFile;

    if (textureFile.find('/') != string::npos || textureFile.find('\\') != string::npos) {
        return directoryOf(currentConfigFile) + textureFile;
    }

    return directoryOf(currentConfigFile) + "textures/" + textureFile;
}

// ============================================================================
// GENERIC DYNAMIC SHADER SYSTEM
// ============================================================================

struct DynamicShader {
    GLuint programId;
    std::vector<TextureLayer> layers;
};

static std::map<std::string, DynamicShader> gDynamicShaders;
static GLfloat gShaderLightPosEye[4] = {0.0f, 0.0f, 0.0f, 1.0f};

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        vector<char> log(max(1, logLen));
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        cerr << "Shader compile error: " << log.data() << endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static void transformPointByMatrix(const GLfloat m[16], const GLfloat in[4], GLfloat out[4]) {
    out[0] = m[0] * in[0] + m[4] * in[1] + m[8]  * in[2] + m[12] * in[3];
    out[1] = m[1] * in[0] + m[5] * in[1] + m[9]  * in[2] + m[13] * in[3];
    out[2] = m[2] * in[0] + m[6] * in[1] + m[10] * in[2] + m[14] * in[3];
    out[3] = m[3] * in[0] + m[7] * in[1] + m[11] * in[2] + m[15] * in[3];
}

static void updateShaderLightFromScene() {
    gShaderLightPosEye[0] = 0.0f;
    gShaderLightPosEye[1] = 0.0f;
    gShaderLightPosEye[2] = 0.0f;
    gShaderLightPosEye[3] = 1.0f;

    const Light* light = nullptr;
    for (const auto& candidate : sceneLights) {
        if (candidate.type == Light::Type::LT_POINT) {
            light = &candidate;
            break;
        }
    }
    if (!light) return;

    GLfloat modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    GLfloat lightWorld[4] = {light->x, light->y, light->z, 1.0f};
    transformPointByMatrix(modelView, lightWorld, gShaderLightPosEye);
}

// Shader key encodes the structural part of each layer (blend/mixFactor/useChannel).
// Opacity is not in the key because it is passed as a per-frame uniform.
static std::string generateShaderKey(const std::vector<TextureLayer>& layers) {
    std::string key;
    for (const auto& l : layers) {
        if (!key.empty()) key += "|";
        key += l.blend + ":" + l.mixFactor + ":" + l.useChannel;
    }
    return key;
}

// Generates a fragment shader driven entirely by the TextureLayer list.
// The engine has no knowledge of what any layer represents — only blend math.
static std::string generateFragmentShader(const std::vector<TextureLayer>& layers) {
    std::stringstream ss;
    int n = (int)layers.size();

    ss << "#version 120\n";
    for (int i = 0; i < n; i++)
        ss << "uniform sampler2D u_tex_" << i << ";\n";
    ss << "uniform vec4  uLightPosEye;\n";
    ss << "uniform vec3  uGlobalAmbient;\n";
    for (int i = 0; i < n; i++)
        ss << "uniform float u_opacity_" << i << ";\n";

    ss << R"GLSL(
varying vec3 vNormalEye;
varying vec3 vPosEye;
varying vec2 vTexCoord;

void main() {
    vec3 N = normalize(vNormalEye);
    vec3 L = (uLightPosEye.w < 0.5)
           ? normalize(uLightPosEye.xyz)
           : normalize(uLightPosEye.xyz - vPosEye);
    vec3 V = normalize(-vPosEye);
    vec3 H = normalize(L + V);
    float ndl        = max(dot(N, L), 0.0);
    float rawSpec    = pow(max(dot(N, H), 0.0), max(gl_FrontMaterial.shininess, 1.0));
    float specFactor = rawSpec * sign(ndl);

    vec3 color = gl_FrontMaterial.ambient.rgb  * uGlobalAmbient
               + gl_FrontMaterial.diffuse.rgb  * ndl
               + gl_FrontMaterial.specular.rgb * specFactor
               + gl_FrontMaterial.emission.rgb;
)GLSL";

    for (int i = 0; i < n; i++) {
        const auto& l = layers[i];
        ss << "\n    // --- layer " << i << " role=\"" << l.role
           << "\" blend=" << l.blend << " mixFactor=" << l.mixFactor
           << " useChannel=" << l.useChannel << "\n";

        // sample
        ss << "    vec4 raw" << i << " = texture2D(u_tex_" << i << ", vTexCoord);\n";

        // channel extraction
        std::string ch = l.useChannel.empty() ? "rgb" : l.useChannel;
        if      (ch == "r") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".r);\n";
        else if (ch == "g") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".g);\n";
        else if (ch == "b") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".b);\n";
        else if (ch == "a") ss << "    vec3 samp" << i << " = vec3(raw" << i << ".a);\n";
        else                ss << "    vec3 samp" << i << " = raw" << i << ".rgb;\n";

        // mixFactor
        std::string mf = l.mixFactor.empty() ? "1.0" : l.mixFactor;
        if      (mf == "ndl")      ss << "    float fac" << i << " = ndl;\n";
        else if (mf == "1-ndl")    ss << "    float fac" << i << " = 1.0 - ndl;\n";
        else if (mf == "specular") ss << "    float fac" << i << " = specFactor;\n";
        else                       ss << "    float fac" << i << " = " << mf << ";\n";

        // blend operation
        std::string w = "fac" + std::to_string(i) + " * u_opacity_" + std::to_string(i);
        std::string bm = l.blend.empty() ? "mix" : l.blend;
        if (bm == "multiply") {
            ss << "    color = mix(color, color * samp" << i << ", " << w << ");\n";
        } else if (bm == "add") {
            ss << "    color += samp" << i << " * " << w << ";\n";
        } else if (bm == "overlay") {
            ss << "    vec3 ov" << i << " = mix(2.0*color*samp" << i
               << ", 1.0-2.0*(1.0-color)*(1.0-samp" << i << "), step(0.5, color));\n";
            ss << "    color = mix(color, ov" << i << ", " << w << ");\n";
        } else { // mix / replace / default
            ss << "    color = mix(color, samp" << i << ", " << w << ");\n";
        }
    }

    ss << "\n    gl_FragColor = vec4(clamp(color, 0.0, 1.0), gl_FrontMaterial.diffuse.a);\n}\n";
    return ss.str();
}

static GLuint compileDynamicShader(const std::vector<TextureLayer>& layers) {
    static const char* vertexSrc = R"GLSL(
#version 120
varying vec3 vNormalEye;
varying vec3 vPosEye;
varying vec2 vTexCoord;

void main() {
    vNormalEye = normalize(gl_NormalMatrix * gl_Normal);
    vec4 posEye = gl_ModelViewMatrix * gl_Vertex;
    vPosEye = posEye.xyz;
    vTexCoord = gl_MultiTexCoord0.st;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)GLSL";

    std::string fragmentSrc = generateFragmentShader(layers);

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc.c_str());

    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        vector<char> log(max(1, logLen));
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        cerr << "Shader link error: " << log.data() << endl;
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

static bool useDynamicShader(const Model& m) {
    const auto& layers = m.textureLayers;
    if (layers.empty()) return false;

    std::string shaderKey = generateShaderKey(layers);

    if (gDynamicShaders.count(shaderKey) == 0) {
        GLuint program = compileDynamicShader(layers);
        if (!program) return false;
        DynamicShader ds;
        ds.programId = program;
        ds.layers    = layers;
        gDynamicShaders[shaderKey] = ds;
    }

    DynamicShader& shader = gDynamicShaders[shaderKey];
    glUseProgram(shader.programId);

    // Global uniforms
    GLint lightLoc = glGetUniformLocation(shader.programId, "uLightPosEye");
    GLint ambLoc   = glGetUniformLocation(shader.programId, "uGlobalAmbient");
    glUniform4fv(lightLoc, 1, gShaderLightPosEye);
    glUniform3f(ambLoc, sceneGlobalAmbient[0], sceneGlobalAmbient[1], sceneGlobalAmbient[2]);

    // Per-layer: texture sampler + opacity uniform
    for (int i = 0; i < (int)layers.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint texId = loadTextureFromFile(resolveTexturePath(layers[i].file));
        glBindTexture(GL_TEXTURE_2D, texId);

        std::string sampName = "u_tex_"     + std::to_string(i);
        std::string opName   = "u_opacity_" + std::to_string(i);
        glUniform1i(glGetUniformLocation(shader.programId, sampName.c_str()), i);
        glUniform1f(glGetUniformLocation(shader.programId, opName.c_str()),   layers[i].opacity);
    }

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    return true;
}

static void stopDynamicShader(int layerCount) {
    glUseProgram(0);
    for (int i = 0; i < layerCount; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
}

// ============================================================================
// FPS COUNTER
// ============================================================================

int frameCount = 0;
unsigned long lastTime = 0;
std::string frameLogOutputFile = "log/output.csv";

struct FrameSample {
    int frame;
    unsigned long timestampMs;
    float frameTimeMs;
};

static std::vector<FrameSample> gFrameSamples;
static bool gFrameLogRegistered = false;
static bool gFrameLogFlushed = false;

static bool ensureDirectoryExists(const std::string& directoryPath) {
    if (directoryPath.empty()) return true;

    std::string current;
    if (directoryPath[0] == '/' || directoryPath[0] == '\\') {
        current.push_back(directoryPath[0]);
    }

    size_t start = current.empty() ? 0 : 1;
    while (start <= directoryPath.size()) {
        size_t sep = directoryPath.find_first_of("/\\", start);
        std::string part = directoryPath.substr(start, sep - start);
        if (!part.empty()) {
            if (!current.empty() && current.back() != '/') {
                current.push_back('/');
            }
            current += part;
            if (mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
                return false;
            }
        }
        if (sep == std::string::npos) break;
        start = sep + 1;
    }

    return true;
}

static bool ensureParentDirectoryForFile(const std::string& filePath) {
    size_t pos = filePath.find_last_of("/\\");
    if (pos == std::string::npos) return true;
    return ensureDirectoryExists(filePath.substr(0, pos));
}

void flushFrameLog() {
    if (gFrameLogFlushed) return;

    if (!ensureParentDirectoryForFile(frameLogOutputFile)) {
        cerr << "FrameLog: failed to create output directory for '" << frameLogOutputFile << "'" << endl;
        return;
    }

    ofstream logFile(frameLogOutputFile);
    if (!logFile.is_open()) {
        cerr << "FrameLog: failed to open output file '" << frameLogOutputFile << "'" << endl;
        return;
    }

    double sum = 0.0;
    for (const auto& sample : gFrameSamples) {
        sum += sample.frameTimeMs;
    }
    double avg = gFrameSamples.empty() ? 0.0 : (sum / gFrameSamples.size());

    logFile << "average_frametime_ms," << fixed << setprecision(3) << avg << "\n";
    logFile << "frame,timestamp_ms,frametime_ms\n";

    for (const auto& sample : gFrameSamples) {
        logFile << sample.frame << ","
                << sample.timestampMs << ","
                << fixed << setprecision(3) << sample.frameTimeMs << "\n";
    }

    gFrameLogFlushed = true;
}

void updateFPS() {
    static unsigned long lastFrameTime = 0;

    if (enableFrameLog && !gFrameLogRegistered) {
        atexit(flushFrameLog);
        gFrameLogRegistered = true;
    }

    unsigned long now = glutGet(GLUT_ELAPSED_TIME);

    if (lastFrameTime > 0 && enableFrameLog) {
        frameTime = (float)(now - lastFrameTime);
        gFrameSamples.push_back({frameCount, now, frameTime});

        if (frameLogMaxRecords > 0 && (int)gFrameSamples.size() >= frameLogMaxRecords) {
            flushFrameLog();
            enableFrameLog = false;
            cout << "FrameLog: captured " << frameLogMaxRecords
                 << " records and saved to '" << frameLogOutputFile << "'." << endl;
        }
    }
    lastFrameTime = now;

    // Average FPS (same as you had)
    frameCount++;
    if (now - lastTime >= 1000) {
        fps = frameCount * 1000.0f / (now - lastTime);
        frameCount = 0;
        lastTime = now;
    }
}

// ============================================================================
// GROUP RENDERING
// ============================================================================

// Catmull-Rom matrix
float M[4][4] = {
    {-0.5f,  1.5f, -1.5f,  0.5f},
    { 1.0f, -2.5f,  2.0f, -0.5f},
    {-0.5f,  0.0f,  0.5f,  0.0f},
    { 0.0f,  1.0f,  0.0f,  0.0f}
};

// Evaluates the Catmull-Rom curve and optionally the tangent at point t in [0,1]
static void catmullRomPoint(const vector<array<float,3>>& pts, float t,
                             float* pos, float* deriv) {

    int numSegments = pts.size() - 3;
    float tScaled   = t * numSegments;
    int   seg       = (int) tScaled;
    if (seg >= numSegments) seg = numSegments - 1;  // clamp to the last segment
    float tLocal    = tScaled - seg;

    // The 4 control points of this segment
    auto& P0 = pts[seg];
    auto& P1 = pts[seg + 1];
    auto& P2 = pts[seg + 2];
    auto& P3 = pts[seg + 3];

    float T[4]  = { tLocal*tLocal*tLocal, tLocal*tLocal, tLocal, 1.0f };
    float Td[4] = { 3*tLocal*tLocal,      2*tLocal,      1.0f,   0.0f }; // derivada

    // For each axis: pos = T * M * P
    for (int axis = 0; axis < 3; axis++) {
        float P[4] = { P0[axis], P1[axis], P2[axis], P3[axis] };

        // MP = M * P
        float MP[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                MP[i] += M[i][j] * P[j];

        // position = T · MP
        pos[axis] = 0;
        for (int i = 0; i < 4; i++)
            pos[axis] += T[i] * MP[i];

        // tangent = Td · MP
        if (deriv) {
            deriv[axis] = 0;
            for (int i = 0; i < 4; i++)
                deriv[axis] += Td[i] * MP[i];
        }
    }
}

static void drawCatmullRomCurve(const vector<array<float,3>>& pts) {
    const int samples = 200;

    glDisable(GL_CULL_FACE);
    glColor3f(0.2f, 0.9f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= samples; ++i) {
        float t = (float)i / (float)samples;
        float pos[3];
        catmullRomPoint(pts, t, pos, nullptr);
        glVertex3f(pos[0], pos[1], pos[2]);
    }
    glEnd();

    if (enableCulling) glEnable(GL_CULL_FACE);
}

void renderGroup(const Group& g) {
    glPushMatrix();

   for (const auto& t : g.transforms) {
        if (t.type == TRANSLATE) {
            if (t.time > 0.0f && t.catmullRomPoints.size() >= 4) {
                if (showCurves) {
                    drawCatmullRomCurve(t.catmullRomPoints);
                }

                // animated translation — calculate position on Catmull-Rom curve
                float elapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float tNorm   = fmod(elapsed, t.time) / t.time;  // [0, 1] cyclic 

                float pos[3], deriv[3];
                catmullRomPoint(t.catmullRomPoints, tNorm, pos,
                                t.align ? deriv : nullptr);

                glTranslatef(pos[0], pos[1], pos[2]);

                if (t.align) {
                    // Build rotation matrix to align with the tangent
                    // X = tangent (direction of movement)
                    // Y = previous up (0,1,0) — will be re-orthogonalized
                    // Z = X × Y

                    // Normalize the tangent → X axis
                    float len = sqrt(deriv[0]*deriv[0] + deriv[1]*deriv[1] + deriv[2]*deriv[2]);
                    if (len > 0.0001f) {
                        deriv[0] /= len; deriv[1] /= len; deriv[2] /= len;
                    }

                    float up[3] = {0.0f, 1.0f, 0.0f};

                    // Z = X × Y
                    float zAxis[3] = {
                        deriv[1]*up[2]   - deriv[2]*up[1],
                        deriv[2]*up[0]   - deriv[0]*up[2],
                        deriv[0]*up[1]   - deriv[1]*up[0]
                    };
                    len = sqrt(zAxis[0]*zAxis[0] + zAxis[1]*zAxis[1] + zAxis[2]*zAxis[2]);
                    if (len > 0.0001f) {
                        zAxis[0] /= len; zAxis[1] /= len; zAxis[2] /= len;
                    }

                    // Y = Z × X  (re-orthogonalization)
                    float yAxis[3] = {
                        zAxis[1]*deriv[2] - zAxis[2]*deriv[1],
                        zAxis[2]*deriv[0] - zAxis[0]*deriv[2],
                        zAxis[0]*deriv[1] - zAxis[1]*deriv[0]
                    };

                    // Column-major matrix for OpenGL (4×4)
                    float rotMatrix[16] = {
                        deriv[0], deriv[1], deriv[2], 0,  // X column
                        yAxis[0], yAxis[1], yAxis[2], 0,  // Y column
                        zAxis[0], zAxis[1], zAxis[2], 0,  // Z column
                        0,        0,        0,        1   // translation
                    };
                    glMultMatrixf(rotMatrix);
                }

            } else {
                // static translate — original behavior
                glTranslatef(t.x, t.y, t.z);
            }

        } else if (t.type == ROTATE) {
            if (t.time > 0.0f) {
                // animated rotation — angle grows with time
                float elapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float angle   = fmod(elapsed, t.time) / t.time * 360.0f;
                glRotatef(angle, t.x, t.y, t.z);
            } else {
                // static rotation — original behavior
                glRotatef(t.angle, t.x, t.y, t.z);
            }

        } else if (t.type == SCALE) {
            glScalef(t.x, t.y, t.z);
        }
    }

    for (const auto& m : g.models) {

        if (!m.cull) glDisable(GL_CULL_FACE);
        // Apply material
        GLfloat matAmbient[4] = {m.ambient[0], m.ambient[1], m.ambient[2], 1.0f};
        GLfloat matDiffuse[4] = {m.diffuse[0], m.diffuse[1], m.diffuse[2], 1.0f};
        GLfloat matSpecular[4] = {m.specular[0], m.specular[1], m.specular[2], 1.0f};
        GLfloat matEmissive[4] = {m.emissive[0], m.emissive[1], m.emissive[2], 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmissive);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m.shininess);

        // Shader path: any model with texture layers uses the dynamic shader.
        // No-texture models fall back to fixed-function with a flat colour.
        bool usedDynamicShader = !m.textureLayers.empty() && useDynamicShader(m);
        if (!usedDynamicShader) {
            glDisable(GL_TEXTURE_2D);
            glColor3f(m.r, m.g, m.b);
        }

        if (!disableVBO) {
            // Upload to GPU if needed
            if (!m.gpuReady || (m.renderMode == DYNAMIC && m.isDirty)) {
                uploadModelToGPU(const_cast<Model&>(m));
            }

            // Render using VBO
            glBindVertexArray(m.vaoId);
            glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
            glBindVertexArray(0);
        } else {
            // Render without VBO (forced fallback)
            glBegin(GL_TRIANGLES);
            for (const auto& v : m.vertices) {
                glNormal3f(v.nx, v.ny, v.nz);
                glTexCoord2f(v.u, v.v);
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        }

        if (usedDynamicShader) {
            stopDynamicShader((int)m.textureLayers.size());
        }
        
        if (!m.cull && enableCulling) glEnable(GL_CULL_FACE);
    }

    for (const auto& child : g.children) {
        renderGroup(child);
    }

    glPopMatrix();
}

// ============================================================================
// MAIN RENDERING
// ============================================================================

void changeSize(int w, int h) {
    if (h == 0) h = 1;
    float ratio = w * 1.0 / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(camera.fov, ratio, camera.nearPlane, camera.farPlane);
    glMatrixMode(GL_MODELVIEW);
}

void renderScene(void) {
    updateFPS();  // Update FPS
    entityCount = 0;  // Reset entity count per frame

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (!freeCamera) {
        camera.posX = sin(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
        camera.posZ = cos(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
        camera.posY = sin(camera.angleBeta * M_PI / 180.0f) * camera.radius;
        camera.lookAtX = 0.0f;
        camera.lookAtY = 0.0f;
        camera.lookAtZ = 0.0f;
    } else {
        // Update lookAt for free camera
        camera.lookAtX = camera.posX + camera.forwardX;
        camera.lookAtY = camera.posY + camera.forwardY;
        camera.lookAtZ = camera.posZ + camera.forwardZ;
        // Update right vector
        camera.rightX = camera.forwardY * camera.upZ - camera.forwardZ * camera.upY;
        camera.rightY = camera.forwardZ * camera.upX - camera.forwardX * camera.upZ;
        camera.rightZ = camera.forwardX * camera.upY - camera.forwardY * camera.upX;
    }
    
    gluLookAt(camera.posX, camera.posY, camera.posZ, 
              camera.lookAtX, camera.lookAtY, camera.lookAtZ,
              camera.upX, camera.upY, camera.upZ);

    updateShaderLightFromScene();

    // Configure light sources after the view transform so they stay fixed in world space.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneGlobalAmbient);

    int lightIdx = 0;
    const int MAX_LIGHTS = 8;
    for (const auto& light : sceneLights) {
        if (lightIdx >= MAX_LIGHTS) break;
        GLenum glLightId = GL_LIGHT0 + lightIdx;
        glEnable(glLightId);

        GLfloat lightColor[4] = {light.r * light.intensity, light.g * light.intensity,
                                 light.b * light.intensity, 1.0f};
        GLfloat lightAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(glLightId, GL_AMBIENT, lightAmbient);
        glLightfv(glLightId, GL_DIFFUSE, lightColor);
        glLightfv(glLightId, GL_SPECULAR, lightColor);

        if (light.type == Light::Type::LT_POINT) {
            GLfloat pos[4] = {light.x, light.y, light.z, 1.0f};
            glLightfv(glLightId, GL_POSITION, pos);
            glLightf(glLightId, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(glLightId, GL_LINEAR_ATTENUATION, 0.0f);
            glLightf(glLightId, GL_QUADRATIC_ATTENUATION, 0.0f);
        } else if (light.type == Light::Type::LT_DIRECTIONAL) {
            GLfloat pos[4] = {light.dirX, light.dirY, light.dirZ, 0.0f};
            glLightfv(glLightId, GL_POSITION, pos);
        } else if (light.type == Light::Type::LT_SPOT) {
            GLfloat pos[4] = {light.x, light.y, light.z, 1.0f};
            glLightfv(glLightId, GL_POSITION, pos);
            GLfloat dir[3] = {light.dirX, light.dirY, light.dirZ};
            glLightfv(glLightId, GL_SPOT_DIRECTION, dir);
            glLightf(glLightId, GL_SPOT_CUTOFF, light.cutoff);
            glLightf(glLightId, GL_SPOT_EXPONENT, 100.0f);
        }

        lightIdx++;
    }

    // Disable unused lights
    for (int i = lightIdx; i < MAX_LIGHTS; i++) {
        glDisable(GL_LIGHT0 + i);
    }

    // Draw axes (only if showAxes is true)
    if (showAxes) {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_LINES);
            glColor3f(1.0f, 0.3f, 0.3f); glVertex3f(-200.0f, 0.0f, 0.0f); glVertex3f(200.0f, 0.0f, 0.0f);
            glColor3f(0.3f, 1.0f, 0.3f); glVertex3f(0.0f, -200.0f, 0.0f); glVertex3f(0.0f, 200.0f, 0.0f);
            glColor3f(0.3f, 0.3f, 1.0f); glVertex3f(0.0f, 0.0f, -200.0f); glVertex3f(0.0f, 0.0f, 200.0f);
        glEnd();
        glEnable(GL_CULL_FACE);
    }

    // Toggle back-face culling
    if (enableCulling) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }

    // Toggle wireframe
    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    renderGroup(rootGroup);

    // Render text for FPS and entity count
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);  // White text

    if (showFPS) {
        glRasterPos2i(10, windowHeight - 20);
        string fpsText = "FPS: " + to_string((int)fps);
        for (char c : fpsText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }
    }

    if (showEntityCount) {
        glRasterPos2i(10, windowHeight - 40);
        string countText = "Entidades: " + to_string(entityCount);
        for (char c : countText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}
