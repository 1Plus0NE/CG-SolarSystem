#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <array>
#include "../include/generator_helpers.h"
#include "../include/figures.h"

using namespace std;

// Bézier basis matrix (cubic)
static float M[4][4] = {
    {-1,  3, -3,  1},
    { 3, -6,  3,  0},
    {-3,  3,  0,  0},
    { 1,  0,  0,  0}
};

// Evaluate a point and its partial derivatives on a bicubic Bézier surface.
// UV coords are the (u, v) parameters themselves — natural parameterization.
static void evalBezier(float u, float v,
                       float Gx[4][4], float Gy[4][4], float Gz[4][4],
                       float& x,    float& y,    float& z,
                       float& dxdu, float& dydu, float& dzdu,
                       float& dxdv, float& dydv, float& dzdv) {
    float U[4]  = { u*u*u, u*u, u, 1.0f };
    float V[4]  = { v*v*v, v*v, v, 1.0f };
    float dU[4] = { 3*u*u, 2*u, 1.0f, 0.0f };
    float dV[4] = { 3*v*v, 2*v, 1.0f, 0.0f };

    float MU[4]={0},MV[4]={0},MdU[4]={0},MdV[4]={0};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            MU[i]  += M[i][j] *  U[j];
            MV[i]  += M[i][j] *  V[j];
            MdU[i] += M[i][j] * dU[j];
            MdV[i] += M[i][j] * dV[j];
        }

    x=y=z=dxdu=dydu=dzdu=dxdv=dydv=dzdv=0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            x    += MU[i]  * Gx[i][j] *  MV[j];
            y    += MU[i]  * Gy[i][j] *  MV[j];
            z    += MU[i]  * Gz[i][j] *  MV[j];
            dxdu += MdU[i] * Gx[i][j] *  MV[j];
            dydu += MdU[i] * Gy[i][j] *  MV[j];
            dzdu += MdU[i] * Gz[i][j] *  MV[j];
            dxdv +=  MU[i] * Gx[i][j] * MdV[j];
            dydv +=  MU[i] * Gy[i][j] * MdV[j];
            dzdv +=  MU[i] * Gz[i][j] * MdV[j];
        }
}

static Vert makeBezierVert(float u, float v,
                           float Gx[4][4], float Gy[4][4], float Gz[4][4]) {
    float x, y, z, dxdu, dydu, dzdu, dxdv, dydv, dzdv;
    evalBezier(u, v, Gx, Gy, Gz, x, y, z, dxdu, dydu, dzdu, dxdv, dydv, dzdv);

    // Normal = dP/du × dP/dv
    float nx = dydu * dzdv - dzdu * dydv;
    float ny = dzdu * dxdv - dxdu * dzdv;
    float nz = dxdu * dydv - dydu * dxdv;
    float len = sqrtf(nx*nx + ny*ny + nz*nz);
    if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
    else             { nx = 0; ny = 1; nz = 0; }

    return {x, y, z, u, v, nx, ny, nz};
}

void generateBezier(const string& patchFile, int tessellation, VertList& out) {
    ifstream in(patchFile);
    if (!in) {
        cerr << "Error: could not open patch file '" << patchFile << "'" << endl;
        return;
    }

    int numPatches;
    in >> numPatches;
    vector<vector<int>> patches(numPatches, vector<int>(16));
    for (int i = 0; i < numPatches; i++) {
        char comma;
        for (int j = 0; j < 16; j++) {
            in >> patches[i][j];
            if (j < 15) in >> comma;
        }
    }

    int numPoints;
    in >> numPoints;
    vector<array<float, 3>> points(numPoints);
    for (int i = 0; i < numPoints; i++) {
        char comma;
        in >> points[i][0] >> comma >> points[i][1] >> comma >> points[i][2];
    }

    for (const auto& patch : patches) {
        float Gx[4][4], Gy[4][4], Gz[4][4];
        for (int i = 0; i < 16; i++) {
            int idx = patch[i], row = i / 4, col = i % 4;
            Gx[row][col] = points[idx][0];
            Gy[row][col] = points[idx][1];
            Gz[row][col] = points[idx][2];
        }

        for (int i = 0; i < tessellation; i++) {
            float u0 = (float) i      / tessellation;
            float u1 = (float)(i + 1) / tessellation;
            for (int j = 0; j < tessellation; j++) {
                float v0 = (float) j      / tessellation;
                float v1 = (float)(j + 1) / tessellation;

                Vert p00 = makeBezierVert(u0, v0, Gx, Gy, Gz);
                Vert p10 = makeBezierVert(u1, v0, Gx, Gy, Gz);
                Vert p01 = makeBezierVert(u0, v1, Gx, Gy, Gz);
                Vert p11 = makeBezierVert(u1, v1, Gx, Gy, Gz);

                // original: generateQuad(p00, p10, p01, p11)
                out.push_back(p00); out.push_back(p10); out.push_back(p11);
                out.push_back(p00); out.push_back(p11); out.push_back(p01);
            }
        }
    }
}
