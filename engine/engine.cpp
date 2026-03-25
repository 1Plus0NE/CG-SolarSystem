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
float frameTime = 0.0f;  // ms entre frames consecutivas

// Window and camera
int windowWidth = 800;
int windowHeight = 600;
Camera camera;
bool freeCamera = false;

// Scene graph and configuration
Group rootGroup;
bool enableFrameLog = false;

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(int argc, char **argv) {
    string configFile;
    for(int i=1; i<argc; i++){
        string arg = argv[i];
        if(arg == "--framelog"){
            enableFrameLog = true;
        } else {
            configFile = arg;
        }
    }
    if(configFile.empty()){
        cerr << "Usage: " << argv[0] << " [--framelog] <config.xml>" << endl;
        return 1;
    }

    // 1. GLUT primeiro — cria o contexto OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("SolariUM - Phase 2");

    // 2. GLEW logo após a janela existir — inicializa ponteiros GL
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "GLEW init failed: " << glewGetErrorString(err) << endl;
        return 1;
    }

    // 3. Só agora é seguro chamar qualquer função GL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    // 4. Carregar config depois do contexto existir
    string configPath = "../../configs/";
    currentConfigFile = configPath + argv[1];
    loadConfigs(currentConfigFile.c_str());


    // 5. Registar callbacks
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processKeys);
    glutMouseFunc(processMouseButtons);
    glutMotionFunc(processMouseMotion);
    glutIdleFunc(renderScene);

    displayMenu();
    glutMainLoop();
    return 0;
}
