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

static ofstream frameLogFile;

void updateFPS() {
    static unsigned long lastFrameTime = 0;
    static ofstream logFile;
    static bool logOpen = false;

    // Abre o ficheiro uma vez
    if (!logOpen && enableFrameLog) {
        logFile.open("frametime_log.csv");
        logFile << "frame,timestamp_ms,frametime_ms\n";
        logOpen = true;
    }

    unsigned long now = glutGet(GLUT_ELAPSED_TIME);

    if (lastFrameTime > 0 && enableFrameLog) {
        frameTime = (float)(now - lastFrameTime);
        logFile << frameCount << ","
                << now << ","
                << fixed << setprecision(3) << frameTime << "\n";
    }
    lastFrameTime = now;

    // FPS médio (igual ao que tinhas)
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

void renderGroup(const Group& g) {
    glPushMatrix();

    for (const auto& t : g.transforms) {
        if (t.type == TRANSLATE) {
            glTranslatef(t.x, t.y, t.z);
        } else if (t.type == ROTATE) {
            glRotatef(t.angle, t.x, t.y, t.z);
        } else if (t.type == SCALE) {
            glScalef(t.x, t.y, t.z);
        }
    }

    for (const auto& m : g.models) {

        // Upload to GPU if needed
        if (!m.gpuReady || (m.renderMode == DYNAMIC && m.isDirty)) {
            uploadModelToGPU(const_cast<Model&>(m));
        }

        if (!m.cull) glDisable(GL_CULL_FACE);
        glColor3f(m.r, m.g, m.b);

        // Render using VBO
        glBindVertexArray(m.vaoId);
        glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
        glBindVertexArray(0);

        // Render without VBO (fallback)
        /**
        glBegin(GL_TRIANGLES);
        for (const auto& v : m.vertices) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
        */
        
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
