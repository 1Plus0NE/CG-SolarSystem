#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// Estrutura para armazenar um vértice
struct Vertex {
    float x, y, z;
};

// Variáveis globais
vector<Vertex> vertices;
float angleX = 0.0f;
float angleY = 0.0f;
float d = 5.0f;
bool wireframeMode = false;

// Lê o arquivo .3d
bool loadModel(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        Vertex v;
        if (iss >> v.x >> v.y >> v.z) {
            vertices.push_back(v);
        }
    }

    file.close();
    cout << "Loaded " << vertices.size() << " vertices (" 
         << vertices.size() / 3 << " triangles)" << endl;
    return true;
}

void renderScene(void) {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set camera
    glLoadIdentity();
    gluLookAt(d * sin(angleY) * cos(angleX), d * sin(angleX), d * cos(angleY) * cos(angleX),
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    // Draw axes for reference
    glBegin(GL_LINES);
        // X axis - Red
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-100.0f, 0.0f, 0.0f);
        glVertex3f(100.0f, 0.0f, 0.0f);
        // Y axis - Green
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -100.0f, 0.0f);
        glVertex3f(0.0f, 100.0f, 0.0f);
        // Z axis - Blue
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -100.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    // Draw model - sempre em wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < vertices.size(); i++) {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();

    // End of frame
    glutSwapBuffers();
}

void processKeys(unsigned char c, int xx, int yy) {
    switch(c) {
        case 'w': case 'W':
            angleX += 0.1f;
            break;
        case 's': case 'S':
            angleX -= 0.1f;
            break;
        case 'a': case 'A':
            angleY -= 0.1f;
            break;
        case 'd': case 'D':
            angleY += 0.1f;
            break;
        case '+':
            d -= 0.5f;
            if (d < 1.0f) d = 1.0f;
            break;
        case '-':
            d += 0.5f;
            break;
        case 'm': case 'M':
            wireframeMode = !wireframeMode;
            break;
        case 'q': case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void processSpecialKeys(int key, int xx, int yy) {
    switch(key) {
        case GLUT_KEY_UP:
            angleX += 0.1f;
            break;
        case GLUT_KEY_DOWN:
            angleX -= 0.1f;
            break;
        case GLUT_KEY_LEFT:
            angleY -= 0.1f;
            break;
        case GLUT_KEY_RIGHT:
            angleY += 0.1f;
            break;
    }
    glutPostRedisplay();
}

void printHelp() {
    cout << "\n=== Engine Controls ===" << endl;
    cout << "W/S or UP/DOWN    : Rotate up/down" << endl;
    cout << "A/D or LEFT/RIGHT : Rotate left/right" << endl;
    cout << "+/-               : Zoom in/out" << endl;
    cout << "M                 : Toggle wireframe mode" << endl;
    cout << "Q                 : Quit" << endl;
    cout << "======================\n" << endl;
}

int main(int argc, char **argv) {
    // Check arguments
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <model.3d>" << endl;
        return 1;
    }

    // Load model
    if (!loadModel(argv[1])) {
        return 1;
    }

    printHelp();

    // Init GLUT and window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM - 3D Model Viewer");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);        // Ativa face culling
    glCullFace(GL_BACK);           // Remove faces de trás
    glFrontFace(GL_CCW);           // Define que faces counter-clockwise são frontais
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Setup projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    // Enter GLUT's main cycle
    glutMainLoop();

    return 0;
}
