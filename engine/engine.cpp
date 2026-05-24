#include <iostream>
#include <string>
#include <cstdlib>
#include "application_state.h"
#include "rendering.h"
#include "config.h"
#include "input.h"
#include "model.h"
#include "menu.h"
#include "log_system.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

static bool hasPathSeparator(const string& path) {
    return path.find('/') != string::npos || path.find('\\') != string::npos;
}

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
        LOG_ERROR("Usage: " << argv[0] << " [options] <config.xml>");
        LOG_ERROR("Options:");
        LOG_ERROR("  --framelog[=output.csv]   Enable frame-time logging (plain names go to log/output.csv)");
        LOG_ERROR("  --framelog output.csv     Enable frame-time logging to custom file");
        LOG_ERROR("  --framelog-count N        Capture exactly N frame-time records");
        LOG_ERROR("  --no-vbo                  Disable VBO uploads and render in immediate mode");
        LOG_ERROR("  --help, -h, help          Show this helper message");
        LOG_ERROR("Examples:");
        LOG_ERROR("  " << argv[0] << " solar_system.xml");
        LOG_ERROR("  " << argv[0] << " --framelog bench.csv solar_system.xml   # writes to log/bench.csv");
        LOG_ERROR("  " << argv[0] << " --framelog-count 1000 --framelog=vbo.csv solar_system.xml");
        LOG_ERROR("  " << argv[0] << " --no-vbo --framelog=novbo.csv solar_system.xml");
        LOG_ERROR("  " << argv[0] << " --framelog=/tmp/run.csv solar_system.xml   # explicit full path");
    }

    if(configFile.empty()){
        return 1;
    }

    if (frameLogMaxRecords == 0) {
        LOG_ERROR("--framelog-count must be > 0");
        return 1;
    }

    // GLUT must initialise first — it creates the OpenGL context.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("SolariUM");

    // GLEW needs the window/context to exist before it can load GL function pointers.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        LOG_ERROR("GLEW init failed: " << glewGetErrorString(err));
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Config must load after the GL context exists (model upload requires GL).
    string configPath = "../../configs/";
    if (configFile.find('/') != string::npos || configFile.find('\\') != string::npos) {
        currentConfigFile = configFile;
    } else {
        currentConfigFile = configPath + configFile;
    }
    loadConfigs(currentConfigFile.c_str());

    if (disableVBO) {
        LOG_INFO("[Render Mode] VBOs disabled (--no-vbo): rendering via immediate mode.");
    }


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
