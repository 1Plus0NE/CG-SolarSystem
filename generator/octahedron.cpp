#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

// Spherical UV for a point on the octahedron surface.
static Vert makeOctVert(float x, float y, float z, float scale, float cx, float cy, float cz) {
    // Centre the point back at origin for UV / normal computation
    float lx = (x - cx) / scale;
    float ly = (y - cy) / scale;
    float lz = (z - cz) / scale;
    float len = sqrtf(lx*lx + ly*ly + lz*lz);
    if (len < 1e-6f) len = 1.0f;
    float nx = lx / len, ny = ly / len, nz = lz / len;
    float u = 1.0f - (atan2f(lz, lx) + M_PI) / (2.0f * M_PI);
    float v = 1.0f - acosf(fmaxf(-1.0f, fminf(1.0f, ly / len))) / M_PI;
    return {x, y, z, u, v, nx, ny, nz};
}

static void fixSeamOct(Vert& a, Vert& b, Vert& c) {
    float umax = fmaxf(fmaxf(a.u, b.u), c.u);
    if (umax - a.u > 0.5f) a.u += 1.0f;
    if (umax - b.u > 0.5f) b.u += 1.0f;
    if (umax - c.u > 0.5f) c.u += 1.0f;
}

void generateOctahedron(VertList& out, float cx, float cy, float cz, float scale) {
    float v[6][3] = {
        { 1*scale+cx,  0*scale+cy,  0*scale+cz},
        {-1*scale+cx,  0*scale+cy,  0*scale+cz},
        { 0*scale+cx,  1*scale+cy,  0*scale+cz},
        { 0*scale+cx, -1*scale+cy,  0*scale+cz},
        { 0*scale+cx,  0*scale+cy,  1*scale+cz},
        { 0*scale+cx,  0*scale+cy, -1*scale+cz}
    };

    // Keep all faces with consistent outward winding.
    // The +Z hemisphere was previously reversed, which made some faces
    // appear swapped when back-face culling was enabled.
    int tris[8][3] = {
        {4,0,2},{4,2,1},{4,1,3},{4,3,0},
        {5,2,0},{5,1,2},{5,3,1},{5,0,3}
    };

    for (auto& t : tris) {
        Vert a = makeOctVert(v[t[0]][0],v[t[0]][1],v[t[0]][2], scale, cx, cy, cz);
        Vert b = makeOctVert(v[t[1]][0],v[t[1]][1],v[t[1]][2], scale, cx, cy, cz);
        Vert c = makeOctVert(v[t[2]][0],v[t[2]][1],v[t[2]][2], scale, cx, cy, cz);
        fixSeamOct(a, b, c);
        out.push_back(a); out.push_back(b); out.push_back(c);
    }
}
