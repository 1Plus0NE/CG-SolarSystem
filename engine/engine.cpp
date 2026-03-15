// ============================================================================
// ENGINE - MAIN APPLICATION
// ============================================================================
// This file now serves as the main entry point with modular organization
// Rendering logic: rendering.cpp
// Input handling:  input.cpp
// Configuration:   config.cpp
// Model loading:   model.cpp
// Data structures: geometry.h
// Menu interface:  menu.cpp
// ============================================================================

#include <iostream>
#include <string>
#include "geometry.h"
#include "rendering.h"
#include "config.h"
#include "input.h"
#include "model.h"
#include "menu.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Rendering flags
bool showAxes = false;
bool enableCulling = true;
bool showFPS = false;
bool showEntityCount = false;
int entityCount = 0;
float fps = 0.0f;
bool wireframeMode = false;

// Window and camera
int windowWidth = 800;
int windowHeight = 600;
Camera camera;
bool freeCamera = false;

// Scene graph and configuration
Group rootGroup;

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <config.xml>" << endl;
        return 1;
    }

    // Load configuration
    string configPath = "../../configs/";
    currentConfigFile = configPath + argv[1];
    loadConfigs(currentConfigFile.c_str());

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("SolariUM - Phase 2");

    // Register callbacks
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processKeys);
    glutMouseFunc(processMouseButtons);
    glutMotionFunc(processMouseMotion);
    // keep rendering continuously so FPS counter updates even when idle
    glutIdleFunc(renderScene);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);



    // Display menu
    displayMenu();

    // Start main loop
    glutMainLoop();
    return 0;
}
