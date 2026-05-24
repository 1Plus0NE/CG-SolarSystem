#include "config.h"
#include "model.h"
#include "log_system.h"
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

// Reads R/G/B attributes (0–255 range) from a child element of parent.
// Existing out[] values are used as defaults (preserves colours set via hex).
static void parseColorRGB(XMLElement* parent, const char* tag, float out[3]) {
    XMLElement* elem = parent->FirstChildElement(tag);
    if (!elem) return;
    out[0] = elem->FloatAttribute("R", out[0] * 255.0f) / 255.0f;
    out[1] = elem->FloatAttribute("G", out[1] * 255.0f) / 255.0f;
    out[2] = elem->FloatAttribute("B", out[2] * 255.0f) / 255.0f;
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
                    // animated: read Catmull-Rom control points
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
                    t.angle = 0.0f;  // computed per frame from elapsed time
                } else {
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


                // Each <texture> element becomes one TextureLayer (file required;
                // role, blend, opacity, mixFactor, useChannel are optional).
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

                // color="#RRGGBB" sets the flat colour and seeds diffuse.
                // A nested <color> block can then override individual components.
                const char* col = modelElem->Attribute("color");
                if (col) {
                    parseHexColor(col, m.r, m.g, m.b);
                    m.diffuse[0] = m.r; m.diffuse[1] = m.g; m.diffuse[2] = m.b;
                }

                XMLElement* colorElem = modelElem->FirstChildElement("color");
                if (colorElem) {
                    parseColorRGB(colorElem, "diffuse",  m.diffuse);
                    parseColorRGB(colorElem, "ambient",  m.ambient);
                    parseColorRGB(colorElem, "specular", m.specular);
                    parseColorRGB(colorElem, "emissive", m.emissive);
                    XMLElement* diff = colorElem->FirstChildElement("diffuse");
                    if (diff) m.alpha = diff->FloatAttribute("A", 1.0f);
                    XMLElement* shin = colorElem->FirstChildElement("shininess");
                    if (shin) m.shininess = shin->FloatAttribute("value", 0.0f);
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
        LOG_ERROR("Failed to load XML file: " << filename);
        return;
    }

    XMLElement* root = doc.FirstChildElement("world");
    if (!root) {
        LOG_ERROR("No 'world' element found in " << filename);
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

        // Derive orbital parameters from the loaded Cartesian position so that
        // the orbital camera starts in the correct orientation.
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
        LOG_INFO("Global ambient: "
             << sceneGlobalAmbient[0] << ", "
             << sceneGlobalAmbient[1] << ", "
             << sceneGlobalAmbient[2] << ", "
             << sceneGlobalAmbient[3]);
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

        // Helper: reads an attribute trying camelCase first, then lowercase fallback.
        auto lightAttr = [](XMLElement* e, const char* camel, const char* lower, float def) -> float {
            const char* v = e->Attribute(camel);
            if (!v) v = e->Attribute(lower);
            return v ? (float)atof(v) : def;
        };

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
                L.x = lightAttr(lightElem, "posX", "posx", 0.0f);
                L.y = lightAttr(lightElem, "posY", "posy", 0.0f);
                L.z = lightAttr(lightElem, "posZ", "posz", 0.0f);
            } else if (type == "directional") {
                L.type = Light::Type::LT_DIRECTIONAL;
                L.dirX = lightAttr(lightElem, "dirX", "dirx", 0.0f);
                L.dirY = lightAttr(lightElem, "dirY", "diry", -1.0f);
                L.dirZ = lightAttr(lightElem, "dirZ", "dirz", 0.0f);
            } else { // spotlight/spot
                L.type = Light::Type::LT_SPOT;
                L.x = lightAttr(lightElem, "posX", "posx", 0.0f);
                L.y = lightAttr(lightElem, "posY", "posy", 0.0f);
                L.z = lightAttr(lightElem, "posZ", "posz", 0.0f);
                L.dirX = lightAttr(lightElem, "dirX", "dirx", 0.0f);
                L.dirY = lightAttr(lightElem, "dirY", "diry", -1.0f);
                L.dirZ = lightAttr(lightElem, "dirZ", "dirz", 0.0f);
                L.cutoff = lightElem->FloatAttribute("cutoff", 45.0f);
            }

            sceneLights.push_back(L);
            lightElem = lightElem->NextSiblingElement("light");
        }
    }

    LOG_INFO("Configuration loaded: " << filename);
}

void reloadConfig() {
    rootGroup = Group();
    clearModelCache();
    loadConfigs(currentConfigFile.c_str());
    LOG_INFO("Configuration reloaded.");
    glutPostRedisplay();
}
