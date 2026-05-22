#pragma once
#include <array>
#include <cmath>

// Shared vector/matrix utilities used by model.cpp and rendering.cpp.

using Vec3 = std::array<float, 3>;
using Vec2 = std::array<float, 2>;

inline Vec3 subtract(const Vec3& a, const Vec3& b) {
    return {a[0]-b[0], a[1]-b[1], a[2]-b[2]};
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a[1]*b[2] - a[2]*b[1],
        a[2]*b[0] - a[0]*b[2],
        a[0]*b[1] - a[1]*b[0]
    };
}

inline float vec3_length(const Vec3& v) {
    return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// Normalizes v in-place; falls back to (0,0,1) for near-zero vectors.
inline void normalize(Vec3& v) {
    float len = vec3_length(v);
    if (len > 1e-6f) { v[0]/=len; v[1]/=len; v[2]/=len; }
    else              v = {0.0f, 0.0f, 1.0f};
}

// Variants that operate on raw float[3] (used in rendering.cpp alignment code).
inline void normalize3(float v[3]) {
    float len = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len > 1e-4f) { v[0]/=len; v[1]/=len; v[2]/=len; }
}

inline void cross3(const float a[3], const float b[3], float out[3]) {
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}

// Column-major 4×4 matrix × homogeneous point.
inline void mat4_transform_point(const float m[16], const float in[4], float out[4]) {
    out[0] = m[0]*in[0] + m[4]*in[1] + m[8] *in[2] + m[12]*in[3];
    out[1] = m[1]*in[0] + m[5]*in[1] + m[9] *in[2] + m[13]*in[3];
    out[2] = m[2]*in[0] + m[6]*in[1] + m[10]*in[2] + m[14]*in[3];
    out[3] = m[3]*in[0] + m[7]*in[1] + m[11]*in[2] + m[15]*in[3];
}
