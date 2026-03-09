#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <map>
#include <tinyxml2.h>
#include <cstdlib>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
using namespace tinyxml2;

struct Vertex {
    float x, y, z;
};

enum TransformType { TRANSLATE, ROTATE, SCALE };

struct Transform {
    TransformType type;
    float x, y, z;
    float angle;   // Static angle for ROTATE.
};

struct Model {
    string file;
    list<Vertex> vertices;
    float r, g, b; // display color (default white)
    Model() : r(1.0f), g(1.0f), b(1.0f) {}
};

// Parse a #RRGGBB hex string into [0,1] floats
void parseHexColor(const char* hex, float& r, float& g, float& b) {
    if (!hex || hex[0] != '#' || strlen(hex) < 7) {
        r = g = b = 1.0f;
        return;
    }
    unsigned int ri, gi, bi;
    sscanf(hex + 1, "%02x%02x%02x", &ri, &gi, &bi);
    r = ri / 255.0f;
    g = gi / 255.0f;
    b = bi / 255.0f;
}

struct Group {
    list<Transform> transforms;
    list<Model> models;
    list<Group> children;
};

struct Camera {
    float posX, posY, posZ;
    float lookAtX, lookAtY, lookAtZ;
    float upX, upY, upZ;
    float fov, nearPlane, farPlane;
    float angleAlfa, angleBeta, radius;
};

// Global variables
Group rootGroup;
map<string, list<Vertex>> modelCache;
int windowWidth = 800;
int windowHeight = 600;
Camera camera;

// Mouse tracking variables
int mouseX, mouseY;
bool mousePressed = false;
bool wireframeMode = false; // Toggle with 'f' key

// Stars
struct Star {
    float x, y, z;
    float brightness;
};
vector<Star> stars;
int numStars = 2000;
float starSphereRadius = 500.0f;

void generateStars() {
    srand(42);
    stars.clear();
    for (int i = 0; i < numStars; i++) {
        float u = (float)rand() / RAND_MAX;
        float v = (float)rand() / RAND_MAX;
        float theta = 2.0f * M_PI * u;
        float phi = acos(2.0f * v - 1.0f);
        Star s;
        s.x = starSphereRadius * sin(phi) * cos(theta);
        s.y = starSphereRadius * sin(phi) * sin(theta);
        s.z = starSphereRadius * cos(phi);
        s.brightness = 0.5f + 0.5f * ((float)rand() / RAND_MAX);
        stars.push_back(s);
    }
}

void renderStars() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (const auto& s : stars) {
        glColor3f(s.brightness, s.brightness, s.brightness * 0.95f);
        glVertex3f(s.x + camera.lookAtX, s.y + camera.lookAtY, s.z + camera.lookAtZ);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

/**
 * Load a .3d model file
 */
list<Vertex> loadModelFile(const char* filename) {
    list<Vertex> vertices;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open model file " << filename << endl;
        return vertices;
    }

    string line;
    int expectedCount = -1;
    if (getline(file, line)) {
        istringstream iss(line);
        iss >> expectedCount;
    }

    int loadedCount = 0;
    while (getline(file, line)) {
        istringstream iss(line);
        Vertex v;
        if (iss >> v.x >> v.y >> v.z) {
            vertices.push_back(v);
            loadedCount++;
        }
    }

    file.close();
    return vertices;
}

/**
 * Get model vertices (with caching)
 */
list<Vertex> getModelVertices(const string& filename) {
    if (modelCache.find(filename) == modelCache.end()) {
        string modelPath = "../../figures/";
        modelPath += filename;
        modelCache[filename] = loadModelFile(modelPath.c_str());
    }
    return modelCache[filename];
}

/**
 * Parse a group element recursively
 */
Group parseGroup(XMLElement* groupElem) {
    Group g;

    // Parse transforms
    XMLElement* transformElem = groupElem->FirstChildElement("transform");
    if (transformElem) {
        XMLElement* child = transformElem->FirstChildElement();
        while (child) {
            string name = child->Name();
            Transform t;
            if (name == "translate") {
                t.type = TRANSLATE;
                t.x = child->FloatAttribute("x", 0.0f);
                t.y = child->FloatAttribute("y", 0.0f);
                t.z = child->FloatAttribute("z", 0.0f);
                g.transforms.push_back(t);
            } else if (name == "rotate") {
                t.type = ROTATE;
                t.angle = child->FloatAttribute("angle", 0.0f);
                t.x = child->FloatAttribute("x", 0.0f);
                t.y = child->FloatAttribute("y", 0.0f);
                t.z = child->FloatAttribute("z", 0.0f);
                g.transforms.push_back(t);
            } else if (name == "scale") {
                t.type = SCALE;
                t.x = child->FloatAttribute("x", 1.0f);
                t.y = child->FloatAttribute("y", 1.0f);
                t.z = child->FloatAttribute("z", 1.0f);
                g.transforms.push_back(t);
            }
            child = child->NextSiblingElement();
        }
    }

    // Parse models
    XMLElement* modelsElem = groupElem->FirstChildElement("models");
    if (modelsElem) {
        XMLElement* modelElem = modelsElem->FirstChildElement("model");
        while (modelElem) {
            const char* file = modelElem->Attribute("file");
            if (file) {
                Model m;
                m.file = file;
                m.vertices = getModelVertices(file);
                const char* col = modelElem->Attribute("color");
                parseHexColor(col, m.r, m.g, m.b);
                g.models.push_back(m);
            }
            modelElem = modelElem->NextSiblingElement("model");
        }
    }

    // Parse sub-groups
    XMLElement* subGroupElem = groupElem->FirstChildElement("group");
    while (subGroupElem) {
        g.children.push_back(parseGroup(subGroupElem));
        subGroupElem = subGroupElem->NextSiblingElement("group");
    }

    return g;
}

/**
 * Load XML configuration file
 */
void loadConfigs(const char* filename) {
    XMLDocument doc;

    if (doc.LoadFile(filename) != XML_SUCCESS) {
        cerr << "Error loading XML file: " << filename << endl;
        return;
    }

    XMLElement* root = doc.FirstChildElement("world");
    if (!root) {
        cerr << "Error: No 'world' element found" << endl;
        return;
    }

    // Window
    XMLElement* window = root->FirstChildElement("window");
    if (window) {
        windowWidth = window->IntAttribute("width", 800);
        windowHeight = window->IntAttribute("height", 600);
    }

    // Camera
    XMLElement* cameraElem = root->FirstChildElement("camera");
    if (cameraElem) {
        XMLElement* pos = cameraElem->FirstChildElement("position");
        if (pos) {
            camera.posX = pos->FloatAttribute("x", 10.0f);
            camera.posY = pos->FloatAttribute("y", 10.0f);
            camera.posZ = pos->FloatAttribute("z", 10.0f);
        }
        XMLElement* lookAt = cameraElem->FirstChildElement("lookAt");
        if (lookAt) {
            camera.lookAtX = lookAt->FloatAttribute("x", 0.0f);
            camera.lookAtY = lookAt->FloatAttribute("y", 0.0f);
            camera.lookAtZ = lookAt->FloatAttribute("z", 0.0f);
        }
        XMLElement* up = cameraElem->FirstChildElement("up");
        if (up) {
            camera.upX = up->FloatAttribute("x", 0.0f);
            camera.upY = up->FloatAttribute("y", 1.0f);
            camera.upZ = up->FloatAttribute("z", 0.0f);
        }
        XMLElement* proj = cameraElem->FirstChildElement("projection");
        if (proj) {
            camera.fov = proj->FloatAttribute("fov", 60.0f);
            camera.nearPlane = proj->FloatAttribute("near", 1.0f);
            camera.farPlane = proj->FloatAttribute("far", 1000.0f);
        }

        float dx = camera.posX - camera.lookAtX;
        float dy = camera.posY - camera.lookAtY;
        float dz = camera.posZ - camera.lookAtZ;
        camera.radius = sqrt(dx * dx + dy * dy + dz * dz);
        camera.angleBeta = asin(dy / camera.radius) * 180.0f / M_PI;
        camera.angleAlfa = atan2(dx, dz) * 180.0f / M_PI;
    }

    // Root groups
    XMLElement* groupElem = root->FirstChildElement("group");
    while (groupElem) {
        rootGroup.children.push_back(parseGroup(groupElem));
        groupElem = groupElem->NextSiblingElement("group");
    }

    cout << "Configuration loaded successfully!" << endl;
}

// ============================================================================
// RENDERING
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
        glColor3f(m.r, m.g, m.b);
        glBegin(GL_TRIANGLES);
        for (const auto& v : m.vertices) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    for (const auto& child : g.children) {
        renderGroup(child);
    }

    glPopMatrix();
}

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.posX = sin(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
    camera.posZ = cos(camera.angleAlfa * M_PI / 180.0f) * cos(camera.angleBeta * M_PI / 180.0f) * camera.radius;
    camera.posY = sin(camera.angleBeta * M_PI / 180.0f) * camera.radius;
    
    gluLookAt(camera.posX + camera.lookAtX, camera.posY + camera.lookAtY, camera.posZ + camera.lookAtZ, 
              camera.lookAtX, camera.lookAtY, camera.lookAtZ,
              camera.upX, camera.upY, camera.upZ);

    // Draw stars background
    renderStars();

    // Draw axes
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_LINES);
        glColor3f(1.0f, 0.3f, 0.3f); glVertex3f(-200.0f, 0.0f, 0.0f); glVertex3f(200.0f, 0.0f, 0.0f);
        glColor3f(0.3f, 1.0f, 0.3f); glVertex3f(0.0f, -200.0f, 0.0f); glVertex3f(0.0f, 200.0f, 0.0f);
        glColor3f(0.3f, 0.3f, 1.0f); glVertex3f(0.0f, 0.0f, -200.0f); glVertex3f(0.0f, 0.0f, 200.0f);
    glEnd();
    glEnable(GL_CULL_FACE);

    if (wireframeMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    renderGroup(rootGroup);

    glutSwapBuffers();
}

// ============================================================================
// INPUT PROCESSING
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
        case 'f': wireframeMode = !wireframeMode; break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

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

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <config.xml>" << endl;
        return 1;
    }

    string configPath = "../../configs/";
    loadConfigs((configPath + argv[1]).c_str());

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("SolariUM - Phase 2");

    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processKeys);
    glutMouseFunc(processMouseButtons);
    glutMotionFunc(processMouseMotion);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    generateStars();

    glutMainLoop();
    return 0;
}
