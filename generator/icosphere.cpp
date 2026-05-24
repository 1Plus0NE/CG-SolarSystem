#include <vector>
#include <cmath>
#include "../include/generator_helpers.h"

using namespace std;

struct IcoPoint {
    float x, y, z;
    IcoPoint(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    IcoPoint normalize(float r) const {
        float l = sqrtf(x*x + y*y + z*z);
        return {x * r / l, y * r / l, z * r / l};
    }
};

// Spherical UV from a point on the sphere:
//   u = (atan2(z, x) + π) / 2π  — longitude in [0, 1]
//   v = 1 - acos(y / r) / π     — colatitude flipped so v=1 at north pole
static void sphereUV(float x, float y, float z, float r, float& u, float& v) {
    u = 1.0f - (atan2f(z, x) + M_PI) / (2.0f * M_PI);
    v = 1.0f - acosf(fmaxf(-1.0f, fminf(1.0f, y / r))) / M_PI;
}

// Fix UV seam: if a triangle straddles the u=0/1 seam (one vertex near 0,
// others near 1), add 1 to the low-side vertices so interpolation is correct.
static void fixSeam(Vert& a, Vert& b, Vert& c) {
    float umax = fmaxf(fmaxf(a.u, b.u), c.u);
    if (umax - a.u > 0.5f) a.u += 1.0f;
    if (umax - b.u > 0.5f) b.u += 1.0f;
    if (umax - c.u > 0.5f) c.u += 1.0f;
}

// Fix UV at poles: if two vertices share x=0,z=0 (pole), inherit u from the
// third vertex so there's no pinch artefact.
static void fixPole(Vert& a, Vert& b, Vert& c, float r) {
    bool aIsPole = (fabsf(a.y) >= r * 0.9999f);
    bool bIsPole = (fabsf(b.y) >= r * 0.9999f);
    bool cIsPole = (fabsf(c.y) >= r * 0.9999f);
    if (aIsPole) a.u = (b.u + c.u) * 0.5f;
    if (bIsPole) b.u = (a.u + c.u) * 0.5f;
    if (cIsPole) c.u = (a.u + b.u) * 0.5f;
}

void generateIcosphere(float radius, int subdivisions, VertList& out) {
    const float t = (1.0f + sqrtf(5.0f)) / 2.0f;
    vector<IcoPoint> pts = {
        IcoPoint(-1,  t,  0).normalize(radius),
        IcoPoint( 1,  t,  0).normalize(radius),
        IcoPoint(-1, -t,  0).normalize(radius),
        IcoPoint( 1, -t,  0).normalize(radius),
        IcoPoint( 0, -1,  t).normalize(radius),
        IcoPoint( 0,  1,  t).normalize(radius),
        IcoPoint( 0, -1, -t).normalize(radius),
        IcoPoint( 0,  1, -t).normalize(radius),
        IcoPoint( t,  0, -1).normalize(radius),
        IcoPoint( t,  0,  1).normalize(radius),
        IcoPoint(-t,  0, -1).normalize(radius),
        IcoPoint(-t,  0,  1).normalize(radius)
    };
    struct Tri { int v1, v2, v3; };
    vector<Tri> faces = {
        {0,11,5},{0,5,1},{0,1,7},{0,7,10},{0,10,11},
        {1,5,9},{5,11,4},{11,10,2},{10,7,6},{7,1,8},
        {3,9,4},{3,4,2},{3,2,6},{3,6,8},{3,8,9},
        {4,9,5},{2,4,11},{6,2,10},{8,6,7},{9,8,1}
    };

    auto midpoint = [&](int p1, int p2) -> int {
        IcoPoint m{(pts[p1].x+pts[p2].x)*0.5f,
                   (pts[p1].y+pts[p2].y)*0.5f,
                   (pts[p1].z+pts[p2].z)*0.5f};
        pts.push_back(m.normalize(radius));
        return (int)pts.size() - 1;
    };

    for (int s = 0; s < subdivisions; s++) {
        vector<Tri> next;
        for (const auto& f : faces) {
            int a = midpoint(f.v1, f.v2);
            int b = midpoint(f.v2, f.v3);
            int c = midpoint(f.v3, f.v1);
            next.push_back({f.v1, a, c});
            next.push_back({f.v2, b, a});
            next.push_back({f.v3, c, b});
            next.push_back({a, b, c});
        }
        faces = next;
    }

    for (const auto& f : faces) {
        auto makeVert = [&](int idx) -> Vert {
            const IcoPoint& p = pts[idx];
            float u, v;
            sphereUV(p.x, p.y, p.z, radius, u, v);
            float len = sqrtf(p.x*p.x + p.y*p.y + p.z*p.z);
            return {p.x, p.y, p.z, u, v, p.x/len, p.y/len, p.z/len};
        };

        Vert va = makeVert(f.v1);
        Vert vb = makeVert(f.v2);
        Vert vc = makeVert(f.v3);

        fixPole(va, vb, vc, radius);
        fixSeam(va, vb, vc);

        out.push_back(va);
        out.push_back(vb);
        out.push_back(vc);
    }
}
