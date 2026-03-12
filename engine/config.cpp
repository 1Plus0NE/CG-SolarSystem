#include "config.h"
#include "model.h"
#include <tinyxml2.h>
#include <iostream>
#include <cstring>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;
using namespace tinyxml2;

string currentConfigFile;

// ============================================================================
// COLOR PARSING
// ============================================================================

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

// ============================================================================
// XML PARSING
// ============================================================================

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

// ============================================================================
// CONFIG LOADING
// ============================================================================

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
        extern int windowWidth;
        extern int windowHeight;
        windowWidth = window->IntAttribute("width", 800);
        windowHeight = window->IntAttribute("height", 600);
    }

    // Camera
    XMLElement* cameraElem = root->FirstChildElement("camera");
    if (cameraElem) {
        extern Camera camera;
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

void reloadConfig() {
    rootGroup = Group();
    clearModelCache();
    loadConfigs(currentConfigFile.c_str());
    cout << "Configuration reloaded!" << endl;
    glutPostRedisplay();
}
