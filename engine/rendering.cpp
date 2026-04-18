#include "rendering.h"
#include "model.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <string>
#include <GL/glew.h>
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
        glColor3f(m.r, m.g, m.b);

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
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
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
