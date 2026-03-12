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
        case 'w': case 'W': toggleWireframe(); break;
        case 'c': case 'C': toggleCulling(); break;
        case 'o': case 'O': toggleShowFPS(); break;
        case 'a': case 'A': toggleShowAxes(); break;
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
    if (button == GLUT_LEFT_BUTTON) {
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
    if (mousePressed) {
        camera.angleAlfa += (x - mouseX) * 0.5f;
        camera.angleBeta += (y - mouseY) * 0.5f;
        if (camera.angleBeta > 89.0f) camera.angleBeta = 89.0f;
        if (camera.angleBeta < -89.0f) camera.angleBeta = -89.0f;
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
}
