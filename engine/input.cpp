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

const float forwardX = 0.0f, forwardY = 0.0f, forwardZ = -1.0f;
const float rightX = 1.0f, rightY = 0.0f, rightZ = 0.0f;

void moveCameraForward(float speed) {
    camera.posX += forwardX * speed;
    camera.posY += forwardY * speed;
    camera.posZ += forwardZ * speed;
    camera.lookAtX += forwardX * speed;
    camera.lookAtY += forwardY * speed;
    camera.lookAtZ += forwardZ * speed;
}

void moveCameraBackward(float speed) {
    moveCameraForward(-speed);
}

void moveCameraLeft(float speed) {
    camera.posX -= rightX * speed;
    camera.posY -= rightY * speed;
    camera.posZ -= rightZ * speed;
    camera.lookAtX -= rightX * speed;
    camera.lookAtY -= rightY * speed;
    camera.lookAtZ -= rightZ * speed;
}

void moveCameraRight(float speed) {
    moveCameraLeft(-speed);
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
        case '+': camera.radius -= zoomStep; if (camera.radius < 1.0f) camera.radius = 1.0f; break;
        case '-': camera.radius += zoomStep; break;
        case 'w': case 'W': if (!freeCamera) toggleWireframe(); else moveCameraForward(20.0f); break;
        case 's': case 'S': if (freeCamera) moveCameraBackward(20.0f); break;
        case 'a': case 'A': if (!freeCamera) toggleShowAxes(); else moveCameraLeft(20.0f); break;
        case 'd': case 'D': if (freeCamera) moveCameraRight(20.0f); break;
        case 'f': case 'F': freeCamera = !freeCamera; if (freeCamera) { camera.lookAtX = camera.posX + forwardX * 1000.0f; camera.lookAtY = camera.posY + forwardY * 1000.0f; camera.lookAtZ = camera.posZ + forwardZ * 1000.0f; } break;
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
    float zoomStep = camera.radius * 0.05f;
    if (zoomStep < 1.0f) zoomStep = 1.0f;
    if (button == 3) { camera.radius -= zoomStep; if (camera.radius < 1.0f) camera.radius = 1.0f; glutPostRedisplay(); }
    if (button == 4) { camera.radius += zoomStep; glutPostRedisplay(); }
}

void processMouseMotion(int x, int y) {
    if (!freeCamera && mousePressed) {
        camera.angleAlfa += (x - mouseX) * 0.1f;
        camera.angleBeta += (y - mouseY) * 0.1f;
        if (camera.angleBeta > 89.0f) camera.angleBeta = 89.0f;
        if (camera.angleBeta < -89.0f) camera.angleBeta = -89.0f;
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
}
