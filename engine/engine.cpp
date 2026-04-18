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
#include <cstdlib>
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

static bool hasPathSeparator(const string& path) {
    return path.find('/') != string::npos || path.find('\\') != string::npos;
}

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Rendering flags
bool showAxes = false;
bool enableCulling = true;
bool showFPS = false;
bool showEntityCount = false;
bool showCurves = false;
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
bool disableVBO = false;
int frameLogMaxRecords = -1;

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(int argc, char **argv) {
    string configFile;
    frameLogOutputFile = "log/frametime_log.csv";
    bool showHelp = false;

    for(int i=1; i<argc; i++){
        string arg = argv[i];
        if (arg == "--help" || arg == "-h" || arg == "help") {
            showHelp = true;
            continue;
        }
        if (arg.rfind("--framelog-count=", 0) == 0) {
            frameLogMaxRecords = atoi(arg.substr(17).c_str());
            continue;
        }
        if (arg == "--framelog-count") {
            if (i + 1 < argc) {
                string nextArg = argv[i + 1];
                if (!nextArg.empty() && nextArg[0] != '-') {
                    frameLogMaxRecords = atoi(nextArg.c_str());
                    i++;
                }
            }
            continue;
        }
        if(arg.rfind("--framelog=", 0) == 0){
            enableFrameLog = true;
            string output = arg.substr(11);
            frameLogOutputFile = hasPathSeparator(output) ? output : ("log/" + output);
        } else if(arg == "--framelog"){
            enableFrameLog = true;
            if (i + 2 < argc) {
                string nextArg = argv[i + 1];
                if (!nextArg.empty() && nextArg[0] != '-') {
                    frameLogOutputFile = hasPathSeparator(nextArg) ? nextArg : ("log/" + nextArg);
                    i++;
                }
            }
        } else if (arg == "--no-vbo") {
            disableVBO = true;
        } else {
            configFile = arg;
        }
    }

    if (showHelp || configFile.empty()) {
        cerr << "Usage: " << argv[0] << " [options] <config.xml>" << endl;
        cerr << "Options:" << endl;
        cerr << "  --framelog[=output.csv]   Enable frame-time logging (plain names go to log/output.csv)" << endl;
        cerr << "  --framelog output.csv     Enable frame-time logging to custom file" << endl;
        cerr << "  --framelog-count N        Capture exactly N frame-time records" << endl;
        cerr << "  --no-vbo                  Disable VBO uploads and render in immediate mode" << endl;
        cerr << "  --help, -h, help          Show this helper message" << endl;
        cerr << "Examples:" << endl;
        cerr << "  " << argv[0] << " solar_system.xml" << endl;
        cerr << "  " << argv[0] << " --framelog bench.csv solar_system.xml   # writes to log/bench.csv" << endl;
        cerr << "  " << argv[0] << " --framelog-count 1000 --framelog=vbo.csv solar_system.xml" << endl;
        cerr << "  " << argv[0] << " --no-vbo --framelog=novbo.csv solar_system.xml" << endl;
        cerr << "  " << argv[0] << " --framelog=/tmp/run.csv solar_system.xml   # explicit full path" << endl;
    }

    if(configFile.empty()){
        return 1;
    }

    if (frameLogMaxRecords == 0) {
        cerr << "Error: --framelog-count must be > 0" << endl;
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
    if (configFile.find('/') != string::npos || configFile.find('\\') != string::npos) {
        currentConfigFile = configFile;
    } else {
        currentConfigFile = configPath + configFile;
    }
    loadConfigs(currentConfigFile.c_str());

    if (disableVBO) {
        cout << "[Render Mode] VBOs disabled (--no-vbo): rendering via immediate mode." << endl;
    }


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
