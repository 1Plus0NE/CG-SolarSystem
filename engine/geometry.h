#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <list>
#include <string>
#include <map>
#include <vector>

using namespace std;

// ============================================================================
// BASIC DATA STRUCTURES
// ============================================================================

struct Vertex {
    float x, y, z;
};

enum TransformType { TRANSLATE, ROTATE, SCALE };

struct Transform {
    TransformType type;
    float x, y, z;
    float angle;   // Static angle for ROTATE
};

struct Model {
    string file;
    list<Vertex> vertices;
    float r, g, b; // display color (default white)
    bool cull;     // enable backface culling (default true)
    Model() : r(1.0f), g(1.0f), b(1.0f), cull(true) {}
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

#endif // GEOMETRY_H
