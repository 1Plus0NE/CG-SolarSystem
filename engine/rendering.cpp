#include "rendering.h"
#include "model.h"
#include "config.h"
#include "math_helpers.h"
#include "log_system.h"
#include "shader_manager.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <string>
#include <GL/glew.h>
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
        LOG_ERROR("FrameLog: failed to create output directory for '" << frameLogOutputFile << "'");
        return;
    }

    ofstream logFile(frameLogOutputFile);
    if (!logFile.is_open()) {
        LOG_ERROR("FrameLog: failed to open output file '" << frameLogOutputFile << "'");
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
            LOG_INFO("FrameLog: captured " << frameLogMaxRecords
                 << " records and saved to '" << frameLogOutputFile << "'.");
        }
    }
    lastFrameTime = now;

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

    auto& P0 = pts[seg];
    auto& P1 = pts[seg + 1];
    auto& P2 = pts[seg + 2];
    auto& P3 = pts[seg + 3];

    float T[4]  = { tLocal*tLocal*tLocal, tLocal*tLocal, tLocal, 1.0f };
    float Td[4] = { 3*tLocal*tLocal,      2*tLocal,      1.0f,   0.0f }; // derivative of T

    // Evaluate pos = T·(M·P) and optionally deriv = Td·(M·P) for each axis.
    for (int axis = 0; axis < 3; axis++) {
        float P[4] = { P0[axis], P1[axis], P2[axis], P3[axis] };

        float MP[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                MP[i] += M[i][j] * P[j];

        pos[axis] = 0;
        for (int i = 0; i < 4; i++)
            pos[axis] += T[i] * MP[i];

        if (deriv) {
            deriv[axis] = 0;
            for (int i = 0; i < 4; i++)
                deriv[axis] += Td[i] * MP[i];
        }
    }
}

static void drawCatmullRomCurve(const vector<array<float,3>>& pts) {
    const int samples = 200;

    glDisable(GL_LIGHTING);
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

    if (!sceneLights.empty()) glEnable(GL_LIGHTING);
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

                float elapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float tNorm   = fmod(elapsed, t.time) / t.time;  // normalised to [0,1], loops

                float pos[3], deriv[3];
                catmullRomPoint(t.catmullRomPoints, tNorm, pos,
                                t.align ? deriv : nullptr);

                glTranslatef(pos[0], pos[1], pos[2]);

                if (t.align) {
                    // Build rotation matrix: X=tangent, Z=X×up, Y=Z×X
                    normalize3(deriv);

                    float up[3] = {0.0f, 1.0f, 0.0f};
                    float zAxis[3]; cross3(deriv, up,    zAxis); normalize3(zAxis);
                    float yAxis[3]; cross3(zAxis,  deriv, yAxis);

                    // Column-major 4×4 for OpenGL
                    float rotMatrix[16] = {
                        deriv[0], deriv[1], deriv[2], 0,
                        yAxis[0], yAxis[1], yAxis[2], 0,
                        zAxis[0], zAxis[1], zAxis[2], 0,
                        0,        0,        0,        1
                    };
                    glMultMatrixf(rotMatrix);
                }

            } else {
                glTranslatef(t.x, t.y, t.z);
            }

        } else if (t.type == ROTATE) {
            if (t.time > 0.0f) {
                float elapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                float angle   = fmod(elapsed, t.time) / t.time * 360.0f;
                glRotatef(angle, t.x, t.y, t.z);
            } else {
                glRotatef(t.angle, t.x, t.y, t.z);
            }

        } else if (t.type == SCALE) {
            glScalef(t.x, t.y, t.z);
        }
    }

    for (const auto& m : g.models) {

        if (!m.cull) glDisable(GL_CULL_FACE);
        if (m.alpha < 0.999f) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
        }
        GLfloat matAmbient[4] = {m.ambient[0], m.ambient[1], m.ambient[2], 1.0f};
        GLfloat matDiffuse[4] = {m.diffuse[0], m.diffuse[1], m.diffuse[2], m.alpha};
        GLfloat matSpecular[4] = {m.specular[0], m.specular[1], m.specular[2], 1.0f};
        GLfloat matEmissive[4] = {m.emissive[0], m.emissive[1], m.emissive[2], 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmissive);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m.shininess);

        // Shader path: any model with texture layers uses the dynamic shader.
        // No-texture models fall back to fixed-function with a flat colour.
        bool usedDynamicShader = ShaderManager::instance().bind(m);
        if (!usedDynamicShader) {
            glDisable(GL_TEXTURE_2D);
            glColor3f(m.r, m.g, m.b);
        }

        if (!disableVBO) {
            if (!m.gpuFailed && (!m.gpuReady || (m.renderMode == DYNAMIC && m.isDirty))) {
                uploadModelToGPU(const_cast<Model&>(m));
            }
            if (m.gpuReady) {
                glBindVertexArray(m.vaoId);
                glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
                glBindVertexArray(0);
            }
        } else {
            // --no-vbo fallback: immediate mode
            glBegin(GL_TRIANGLES);
            for (const auto& v : m.vertices) {
                glNormal3f(v.nx, v.ny, v.nz);
                glTexCoord2f(v.u, v.v);
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        }

        if (usedDynamicShader) {
            ShaderManager::instance().unbind((int)m.textureLayers.size());
        }
        
        if (!m.cull && enableCulling) glEnable(GL_CULL_FACE);
        if (m.alpha < 0.999f) {
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        if (showNormals && !m.vertices.empty()) {
            const float normalScale = 0.05f;
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glColor3f(1.0f, 1.0f, 0.0f);
            glBegin(GL_LINES);
            for (const auto& v : m.vertices) {
                glVertex3f(v.x, v.y, v.z);
                glVertex3f(v.x + v.nx * normalScale,
                           v.y + v.ny * normalScale,
                           v.z + v.nz * normalScale);
            }
            glEnd();
            if (!sceneLights.empty()) glEnable(GL_LIGHTING);
        }
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
    updateFPS();
    entityCount = 0;

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
        camera.lookAtX = camera.posX + camera.forwardX;
        camera.lookAtY = camera.posY + camera.forwardY;
        camera.lookAtZ = camera.posZ + camera.forwardZ;
        // right = forward × up (recomputed each frame since forward can change)
        camera.rightX = camera.forwardY * camera.upZ - camera.forwardZ * camera.upY;
        camera.rightY = camera.forwardZ * camera.upX - camera.forwardX * camera.upZ;
        camera.rightZ = camera.forwardX * camera.upY - camera.forwardY * camera.upX;
    }
    
    gluLookAt(camera.posX, camera.posY, camera.posZ, 
              camera.lookAtX, camera.lookAtY, camera.lookAtZ,
              camera.upX, camera.upY, camera.upZ);

    ShaderManager::instance().updateLightPosition();

    // Configure light sources after the view transform so they stay fixed in world space.
    // If no lights are defined, disable the lighting pipeline so glColor() takes effect
    // (models from earlier phases rely on per-model colour, not materials).
    if (sceneLights.empty()) {
        glDisable(GL_LIGHTING);
    } else {
        glEnable(GL_LIGHTING);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneGlobalAmbient);
    }

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

    for (int i = lightIdx; i < MAX_LIGHTS; i++) {
        glDisable(GL_LIGHT0 + i);
    }

    if (showAxes) {
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_LINES);
            glColor3f(1.0f, 0.3f, 0.3f); glVertex3f(-200.0f, 0.0f, 0.0f); glVertex3f(200.0f, 0.0f, 0.0f);
            glColor3f(0.3f, 1.0f, 0.3f); glVertex3f(0.0f, -200.0f, 0.0f); glVertex3f(0.0f, 200.0f, 0.0f);
            glColor3f(0.3f, 0.3f, 1.0f); glVertex3f(0.0f, 0.0f, -200.0f); glVertex3f(0.0f, 0.0f, 200.0f);
        glEnd();
        if (!sceneLights.empty()) glEnable(GL_LIGHTING);
        glEnable(GL_CULL_FACE);
    }

    if (enableCulling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);

    renderGroup(rootGroup);

    // HUD: switch to 2D ortho projection for text overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 1.0f);

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
