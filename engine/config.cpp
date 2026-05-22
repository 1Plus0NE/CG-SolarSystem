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
float sceneGlobalAmbient[4] = {0.08f, 0.08f, 0.08f, 1.0f};

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
                t.time = child->FloatAttribute("time", 0.0f);

                if (t.time > 0.0f) {
                    // animated translation — read Catmull-Rom points
                    const char* alignAttr = child->Attribute("align");
                    t.align = alignAttr && (string(alignAttr) == "True" || string(alignAttr) == "true");

                    XMLElement* point = child->FirstChildElement("point");
                    while (point) {
                        array<float, 3> p = {
                            point->FloatAttribute("x", 0.0f),
                            point->FloatAttribute("y", 0.0f),
                            point->FloatAttribute("z", 0.0f)
                        };
                        t.catmullRomPoints.push_back(p);
                        point = point->NextSiblingElement("point");
                    }
                } else {
                    // static translation — original behavior
                    t.x = child->FloatAttribute("x", 0.0f);
                    t.y = child->FloatAttribute("y", 0.0f);
                    t.z = child->FloatAttribute("z", 0.0f);
                }
                g.transforms.push_back(t);
            } else if (name == "rotate") {
                t.type = ROTATE;
                t.x = child->FloatAttribute("x", 0.0f);
                t.y = child->FloatAttribute("y", 0.0f);
                t.z = child->FloatAttribute("z", 0.0f);
                t.time = child->FloatAttribute("time", 0.0f);

                if (t.time > 0.0f) {
                    // animation rotation — angle is calculated at runtime
                    t.angle = 0.0f;
                } else {
                    // static rotation — original behavior
                    t.angle = child->FloatAttribute("angle", 0.0f);
                }
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


                // Texture layers — each <texture> becomes one TextureLayer.
                // Attributes: file (required), role, blend, opacity, mixFactor, useChannel.
                XMLElement* texElem = modelElem->FirstChildElement("texture");
                while (texElem) {
                    const char* texFile = texElem->Attribute("file");
                    if (texFile) {
                        TextureLayer layer;
                        layer.file = texFile;
                        const char* roleAttr  = texElem->Attribute("role");
                        const char* blendAttr = texElem->Attribute("blend");
                        const char* mfAttr    = texElem->Attribute("mixFactor");
                        const char* chAttr    = texElem->Attribute("useChannel");
                        if (roleAttr)  layer.role       = roleAttr;
                        if (blendAttr) layer.blend      = blendAttr;
                        if (mfAttr)    layer.mixFactor  = mfAttr;
                        if (chAttr)    layer.useChannel = chAttr;
                        layer.opacity = texElem->FloatAttribute("opacity", 1.0f);
                        m.textureLayers.push_back(layer);
                    }
                    texElem = texElem->NextSiblingElement("texture");
                }

                // Colors: either attribute `color` as hex or nested <color>
                const char* col = modelElem->Attribute("color");
                if (col) {
                    parseHexColor(col, m.r, m.g, m.b);
                    // also set diffuse to this color
                    m.diffuse[0] = m.r; m.diffuse[1] = m.g; m.diffuse[2] = m.b;
                }

                XMLElement* colorElem = modelElem->FirstChildElement("color");
                if (colorElem) {
                    XMLElement* diff = colorElem->FirstChildElement("diffuse");
                    if (diff) {
                        m.diffuse[0] = diff->FloatAttribute("R", m.diffuse[0] * 255.0f) / 255.0f;
                        m.diffuse[1] = diff->FloatAttribute("G", m.diffuse[1] * 255.0f) / 255.0f;
                        m.diffuse[2] = diff->FloatAttribute("B", m.diffuse[2] * 255.0f) / 255.0f;
                    }
                    XMLElement* amb = colorElem->FirstChildElement("ambient");
                    if (amb) {
                        m.ambient[0] = amb->FloatAttribute("R", 0.0f) / 255.0f;
                        m.ambient[1] = amb->FloatAttribute("G", 0.0f) / 255.0f;
                        m.ambient[2] = amb->FloatAttribute("B", 0.0f) / 255.0f;
                    }
                    XMLElement* spec = colorElem->FirstChildElement("specular");
                    if (spec) {
                        m.specular[0] = spec->FloatAttribute("R", 0.0f) / 255.0f;
                        m.specular[1] = spec->FloatAttribute("G", 0.0f) / 255.0f;
                        m.specular[2] = spec->FloatAttribute("B", 0.0f) / 255.0f;
                    }
                    XMLElement* emi = colorElem->FirstChildElement("emissive");
                    if (emi) {
                        m.emissive[0] = emi->FloatAttribute("R", 0.0f) / 255.0f;
                        m.emissive[1] = emi->FloatAttribute("G", 0.0f) / 255.0f;
                        m.emissive[2] = emi->FloatAttribute("B", 0.0f) / 255.0f;
                    }
                    XMLElement* shin = colorElem->FirstChildElement("shininess");
                    if (shin) {
                        m.shininess = shin->FloatAttribute("value", 0.0f);
                    }
                }
                const char* cullAttr = modelElem->Attribute("cull");
                if (cullAttr && strcmp(cullAttr, "false") == 0) {
                    m.cull = false;
                }
                const char* renderAttr = modelElem->Attribute("render");
                if (renderAttr && strcmp(renderAttr, "dynamic") == 0) {
                    m.renderMode = DYNAMIC;
                } else {
                    m.renderMode = STATIC;
                }
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

    sceneGlobalAmbient[0] = 0.08f;
    sceneGlobalAmbient[1] = 0.08f;
    sceneGlobalAmbient[2] = 0.08f;
    sceneGlobalAmbient[3] = 1.0f;

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

    // Global ambient lighting (optional)
    XMLElement* globalAmbientElem = root->FirstChildElement("globalAmbient");
    if (globalAmbientElem) {
        sceneGlobalAmbient[0] = globalAmbientElem->FloatAttribute("r", sceneGlobalAmbient[0]);
        sceneGlobalAmbient[1] = globalAmbientElem->FloatAttribute("g", sceneGlobalAmbient[1]);
        sceneGlobalAmbient[2] = globalAmbientElem->FloatAttribute("b", sceneGlobalAmbient[2]);
        sceneGlobalAmbient[3] = globalAmbientElem->FloatAttribute("a", 1.0f);
        cerr << "Loaded global ambient: "
             << sceneGlobalAmbient[0] << ", "
             << sceneGlobalAmbient[1] << ", "
             << sceneGlobalAmbient[2] << ", "
             << sceneGlobalAmbient[3] << endl;
    }

    // Root groups
    XMLElement* groupElem = root->FirstChildElement("group");
    while (groupElem) {
        rootGroup.children.push_back(parseGroup(groupElem));
        groupElem = groupElem->NextSiblingElement("group");
    }

    // Parse lights (optional)
    XMLElement* lightsElem = root->FirstChildElement("lights");
    if (lightsElem) {
        sceneLights.clear();
        XMLElement* lightElem = lightsElem->FirstChildElement("light");
        while (lightElem) {
            const char* typeAttr = lightElem->Attribute("type");
            string type = typeAttr ? string(typeAttr) : string("point");
            Light L;
            const char* rAttr = lightElem->Attribute("r");
            const char* gAttr = lightElem->Attribute("g");
            const char* bAttr = lightElem->Attribute("b");
            if (rAttr && gAttr && bAttr) {
                L.r = atof(rAttr) / 255.0f;
                L.g = atof(gAttr) / 255.0f;
                L.b = atof(bAttr) / 255.0f;
            }
            L.intensity = lightElem->FloatAttribute("intensity", 1.0f);

            if (type == "point") {
                L.type = Light::Type::LT_POINT;
                L.x = lightElem->FloatAttribute("posX", 0.0f);
                L.y = lightElem->FloatAttribute("posY", 0.0f);
                L.z = lightElem->FloatAttribute("posZ", 0.0f);
            } else if (type == "directional") {
                L.type = Light::Type::LT_DIRECTIONAL;
                L.dirX = lightElem->FloatAttribute("dirX", 0.0f);
                L.dirY = lightElem->FloatAttribute("dirY", -1.0f);
                L.dirZ = lightElem->FloatAttribute("dirZ", 0.0f);
            } else { // spotlight/spot
                L.type = Light::Type::LT_SPOT;
                L.x = lightElem->FloatAttribute("posX", 0.0f);
                L.y = lightElem->FloatAttribute("posY", 0.0f);
                L.z = lightElem->FloatAttribute("posZ", 0.0f);
                L.dirX = lightElem->FloatAttribute("dirX", 0.0f);
                L.dirY = lightElem->FloatAttribute("dirY", -1.0f);
                L.dirZ = lightElem->FloatAttribute("dirZ", 0.0f);
                L.cutoff = lightElem->FloatAttribute("cutoff", 45.0f);
            }

            sceneLights.push_back(L);
            lightElem = lightElem->NextSiblingElement("light");
        }
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
