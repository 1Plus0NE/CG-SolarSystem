#include "input.h"
#include "config.h"
#include "menu.h"
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// Mouse tracking variables
int mouseX, mouseY;
bool mousePressed = false;

// ============================================================================
// CAMERA MOVEMENT FUNCTIONS
// ============================================================================

void moveCameraForward(float delta) {
    camera.posX += camera.forwardX * camera.velocity * delta;
    camera.posY += camera.forwardY * camera.velocity * delta;
    camera.posZ += camera.forwardZ * camera.velocity * delta;
}

void moveCameraBackward(float delta) {
    moveCameraForward(-delta);
}

void moveCameraLeft(float delta) {
    camera.posX -= camera.rightX * camera.velocity * delta;
    camera.posY -= camera.rightY * camera.velocity * delta;
    camera.posZ -= camera.rightZ * camera.velocity * delta;
}

void moveCameraRight(float delta) {
    camera.posX += camera.rightX * camera.velocity * delta;
    camera.posY += camera.rightY * camera.velocity * delta;
    camera.posZ += camera.rightZ * camera.velocity * delta;
}

void rotateCameraYaw(float angle) {
    // Rotate forward around up
    float cosA = cos(angle * M_PI / 180.0f);
    float sinA = sin(angle * M_PI / 180.0f);
    float newFX = camera.forwardX * cosA - camera.forwardZ * sinA;
    float newFZ = camera.forwardX * sinA + camera.forwardZ * cosA;
    camera.forwardX = newFX;
    camera.forwardZ = newFZ;
    // Normalize
    float len = sqrt(camera.forwardX*camera.forwardX + camera.forwardY*camera.forwardY + camera.forwardZ*camera.forwardZ);
    camera.forwardX /= len;
    camera.forwardY /= len;
    camera.forwardZ /= len;
}

void rotateCameraPitch(float angle) {
    // Rotate forward around right, with clamping
    float cosA = cos(angle * M_PI / 180.0f);
    float sinA = sin(angle * M_PI / 180.0f);
    float newFY = camera.forwardY * cosA - camera.forwardZ * sinA;
    float newFZ = camera.forwardY * sinA + camera.forwardZ * cosA;
    // Clamp to avoid inversion
    if (newFY > 0.99f) newFY = 0.99f;
    if (newFY < -0.99f) newFY = -0.99f;
    camera.forwardY = newFY;
    camera.forwardZ = newFZ;
    // Normalize
    float len = sqrt(camera.forwardX*camera.forwardX + camera.forwardY*camera.forwardY + camera.forwardZ*camera.forwardZ);
    camera.forwardX /= len;
    camera.forwardY /= len;
    camera.forwardZ /= len;
}

// ============================================================================
// KEYBOARD INPUT
// ============================================================================

void processKeys(unsigned char c, int xx, int yy) {
    float zoomStep = camera.radius * 0.05f;
    if (zoomStep < 1.0f) zoomStep = 1.0f;
    switch (c) {
        case 'i': if (camera.angleBeta < 89.0f) camera.angleBeta += 5.0f; break;
        case 'k': if (camera.angleBeta > -89.0f) camera.angleBeta -= 5.0f; break;
        case 'j': camera.angleAlfa -= 5.0f; break;
        case 'l': camera.angleAlfa += 5.0f; break;
        case '+': if (freeCamera) camera.velocity *= 1.1f; else { camera.radius -= zoomStep; if (camera.radius < 1.0f) camera.radius = 1.0f; } break;
        case '-': if (freeCamera) camera.velocity *= 0.9f; else camera.radius += zoomStep; break;
        case 'w': case 'W': if (!freeCamera) toggleWireframe(); else moveCameraForward(1.0f); glutPostRedisplay(); break;
        case 's': case 'S': if (freeCamera) { moveCameraBackward(1.0f); glutPostRedisplay(); } break;
        case 'a': case 'A': if (!freeCamera) toggleShowAxes(); else { moveCameraLeft(1.0f); glutPostRedisplay(); } break;
        case 'd': case 'D': if (freeCamera) { moveCameraRight(1.0f); glutPostRedisplay(); } break;
        case 'f': case 'F': freeCamera = !freeCamera; if (freeCamera) { 
            // Initialize forward towards negative Z
            camera.forwardX = 0.0f; camera.forwardY = 0.0f; camera.forwardZ = -1.0f;
            camera.rightX = 1.0f; camera.rightY = 0.0f; camera.rightZ = 0.0f;
        } glutPostRedisplay(); break;
        case 'c': case 'C': toggleCulling(); break;
        case 'o': case 'O': toggleShowFPS(); break;
        case 'm': case 'M': displayMenu(); break;
        case 'r': case 'R': reloadConfig(); break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

// ============================================================================
// MOUSE INPUT
// ============================================================================

void processMouseButtons(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            mousePressed = true; mouseX = x; mouseY = y;
        } else mousePressed = false;
    }
    if (!freeCamera) {
        float zoomStep = camera.radius * 0.05f;
        if (zoomStep < 1.0f) zoomStep = 1.0f;
        if (button == 3) { camera.radius -= zoomStep; if (camera.radius < 1.0f) camera.radius = 1.0f; glutPostRedisplay(); }
        if (button == 4) { camera.radius += zoomStep; glutPostRedisplay(); }
    }
}

void processMouseMotion(int x, int y) {
    if (mousePressed) {
        if (!freeCamera) {
            camera.angleAlfa += (x - mouseX) * 0.1f;
            camera.angleBeta += (y - mouseY) * 0.1f;
            if (camera.angleBeta > 89.0f) camera.angleBeta = 89.0f;
            if (camera.angleBeta < -89.0f) camera.angleBeta = -89.0f;
        } else {
            // Free camera: yaw with X, pitch with Y
            rotateCameraYaw((x - mouseX) * 0.1f);
            rotateCameraPitch((y - mouseY) * 0.1f);
        }
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
}
