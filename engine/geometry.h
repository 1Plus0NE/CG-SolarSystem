#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <list>
#include <string>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <array>  

using namespace std;

// ============================================================================
// BASIC DATA STRUCTURES
// ============================================================================

struct Vertex {
    float x, y, z;
    float nx, ny, nz;  // Normal vector
    float u, v;        // Texture coordinates
    
    Vertex() : x(0), y(0), z(0), nx(0), ny(0), nz(1), u(0), v(0) {}
    Vertex(float x, float y, float z)
        : x(x), y(y), z(z), nx(0), ny(0), nz(1), u(0), v(0) {}
    Vertex(float x, float y, float z,
           float nx, float ny, float nz,
           float u = 0.0f, float v = 0.0f)
        : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), u(u), v(v) {}
};

enum TransformType { TRANSLATE, ROTATE, SCALE };

struct Transform {
    TransformType type;
    float x, y, z;
    float angle;   // Static angle for ROTATE

    // for dynamic animations
    float time;    // animation duration in seconds
    bool align; // for TRANSLATE, whether to orient along the path

    // points for animation paths (only used for TRANSLATE with time > 0)
    vector<array<float, 3>> catmullRomPoints;

    Transform() : type(TRANSLATE), x(0), y(0), z(0), angle(0), time(0), align(false) {}
};

enum RenderMode {
    STATIC,
    DYNAMIC
};

struct Model {
    string file;
    list<Vertex> vertices;
    float r, g, b; // display color (default white)
    bool cull;     // enable backface culling (default true)
    // Texture and material
    string textureFile;
    bool hasTexture;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float emissive[3];
    float shininess;

    // VBO state
    RenderMode renderMode;
    GLuint vaoId;
    GLuint vboId;
    int vertexCount;
    bool isDirty;              // forçar re-upload no próximo frame
    bool gpuReady;             // VBO já foi inicializado

    Model() : r(1.f), g(1.f), b(1.f), cull(true),
              renderMode(STATIC),
              vaoId(0), vboId(0), vertexCount(0),
              isDirty(false), gpuReady(false), textureFile(), hasTexture(false), shininess(0.0f) {
        ambient[0]=ambient[1]=ambient[2]=0.0f;
        diffuse[0]=diffuse[1]=diffuse[2]=1.0f;
        specular[0]=specular[1]=specular[2]=0.0f;
        emissive[0]=emissive[1]=emissive[2]=0.0f;
    }

};

struct Group {
    list<Transform> transforms;
    list<Model> models;
    list<Group> children;
};

// ============================================================================
// CAMERA STRUCTURE
// ============================================================================

struct Camera {
    float posX, posY, posZ;
    float lookAtX, lookAtY, lookAtZ;
    float upX, upY, upZ;
    float fov;
    float nearPlane, farPlane;
    float radius;
    float angleAlfa, angleBeta;
    
    // Free camera vectors
    float forwardX, forwardY, forwardZ;
    float rightX, rightY, rightZ;
    float velocity;
    
    Camera() : 
        posX(10.0f), posY(10.0f), posZ(10.0f),
        lookAtX(0.0f), lookAtY(0.0f), lookAtZ(0.0f),
        upX(0.0f), upY(1.0f), upZ(0.0f),
        fov(60.0f), nearPlane(1.0f), farPlane(1000.0f),
        radius(20.0f), angleAlfa(0.0f), angleBeta(0.0f),
        forwardX(0.0f), forwardY(0.0f), forwardZ(-1.0f),
        rightX(1.0f), rightY(0.0f), rightZ(0.0f),
        velocity(5.0f) {}
};

    // Simple light structure used by configs and scene
    struct Light {
        enum Type { LT_POINT, LT_DIRECTIONAL, LT_SPOT };
        
        Type type;
        float x, y, z;        // position
        float dirX, dirY, dirZ; // direction for directional/spot
        float r, g, b;        // color
        float intensity;      // scalar intensity
        float cutoff;         // spotlight cutoff angle in degrees
        Light() : type(LT_DIRECTIONAL), x(0), y(0), z(1), dirX(0), dirY(0), dirZ(-1), r(1), g(1), b(1), intensity(1.0f), cutoff(180.0f) {}
    };

#endif // GEOMETRY_H
