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

// ============================================================================
// BÉZIER BASIS MATRIX
// ============================================================================
//
// A cubic Bézier curve is defined by 4 control points P0..P3 and a parameter
// t in [0,1]:
//
//   B(t) = (1-t)³·P0 + 3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3
//
// This can be rewritten in matrix form as:
//
//   B(t) = [t³  t²  t  1] · M · [P0 P1 P2 P3]^T
//
// where M is the Bézier basis matrix below.
// Each row of M corresponds to one power of t (t³, t², t, t⁰).
// Each column corresponds to how that power distributes across the 4 control points.
//
// For example, the constant row {1, 0, 0, 0} means that the t⁰ term
// only comes from P0 with weight 1.
//
static float M[4][4] = {
    {-1,  3, -3,  1},   // coefficients for t³
    { 3, -6,  3,  0},   // coefficients for t²
    {-3,  3,  0,  0},   // coefficients for t¹
    { 1,  0,  0,  0}    // coefficients for t⁰
};

// ============================================================================
// evalBezier — evaluates a point on a bicubic Bézier surface
// ============================================================================
//
// A bicubic Bézier SURFACE extends the curve idea to 2 dimensions.
// Instead of 4 control points, we have a 4×4 grid of 16 control points.
// Two parameters u and v (both in [0,1]) navigate the surface:
//   - u moves in the "row" direction
//   - v moves in the "column" direction
//
// The formula for a point on the surface is:
//
//   P(u,v) = U · M · G · M^T · V^T
//
// where:
//   U  = [u³  u²  u  1]          — power vector for u
//   V  = [v³  v²  v  1]          — power vector for v
//   M  = Bézier basis matrix (above)
//   G  = 4×4 matrix of control point coordinates (one per axis x/y/z)
//
// We split this into three steps to avoid building full matrices:
//   MU = M · U   →  the 4 Bernstein basis values for u
//   MV = M · V   →  the 4 Bernstein basis values for v
//   P  = MU^T · G · MV  →  weighted sum of the 16 control points
//
// Each entry MU[i] * MV[j] is the weight of control point G[i][j].
// The final position is just a weighted average of all 16 control points.
//
static void evalBezier(float u, float v,
                       float Gx[4][4], float Gy[4][4], float Gz[4][4],
                       float& x, float& y, float& z) {

    // Build the power vectors U and V
    // U[0]=u³, U[1]=u², U[2]=u, U[3]=1  (same structure for V)
    float U[4] = { u*u*u, u*u, u, 1.0f };
    float V[4] = { v*v*v, v*v, v, 1.0f };

    // Multiply the basis matrix M by each power vector:
    //   MU[i] = sum_j( M[i][j] * U[j] )
    // This gives us the 4 Bernstein polynomial values for parameter u.
    // MU[0] corresponds to the influence of the first row of control points,
    // MU[1] to the second row, and so on. Same logic applies to MV for v.
    float MU[4] = {0, 0, 0, 0};
    float MV[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            MU[i] += M[i][j] * U[j];
            MV[i] += M[i][j] * V[j];
        }

    // Compute the surface point as a double weighted sum over all 16 control points.
    // For each axis (x, y, z) independently:
    //   result = sum_i sum_j ( MU[i] * G[i][j] * MV[j] )
    //
    // MU[i] * MV[j] is the combined weight of the control point at row i, column j.
    // Since x, y, z are independent axes, we apply the same weights to each.
    x = y = z = 0.0f;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            x += MU[i] * Gx[i][j] * MV[j];
            y += MU[i] * Gy[i][j] * MV[j];
            z += MU[i] * Gz[i][j] * MV[j];
        }
}

// ============================================================================
// generateBezier — reads a .patch file and tessellates all Bézier patches
// ============================================================================
//
// .patch file format:
//   Line 1:               number of patches
//   Next N lines:         16 comma-separated control point indices per patch
//   Next line:            number of control points
//   Remaining lines:      x, y, z coordinates of each control point
//
// Tessellation works by dividing the [0,1]×[0,1] (u,v) parameter space into
// a grid of (tessellation × tessellation) cells. Each cell becomes 2 triangles.
// Higher tessellation = smoother surface, more triangles.
//
void generateBezier(const string& patchFile, int tessellation, list<string>& vertices) {
    ifstream in(patchFile);
    if (!in) {
        cerr << "Error: could not open patch file '" << patchFile << "'" << endl;
        return;
    }

    // -------------------------------------------------------------------------
    // Step 1: Read patch definitions
    // Each patch is described by 16 indices into the control point array.
    // The indices are laid out as a 4×4 grid (row-major):
    //   [ 0  1  2  3 ]
    //   [ 4  5  6  7 ]
    //   [ 8  9 10 11 ]
    //   [12 13 14 15 ]
    // -------------------------------------------------------------------------
    int numPatches;
    in >> numPatches;

    vector<vector<int>> patches(numPatches, vector<int>(16));
    for (int i = 0; i < numPatches; i++) {
        char comma;
        for (int j = 0; j < 16; j++) {
            in >> patches[i][j];
            if (j < 15) in >> comma; // consume the comma separator between indices
        }
    }

    // -------------------------------------------------------------------------
    // Step 2: Read control points
    // Each control point is a 3D position (x, y, z).
    // Control points are shared between patches — that is why patches store
    // indices instead of coordinates directly. This saves memory and ensures
    // that adjacent patches share the exact same boundary points, keeping
    // the surface watertight (no cracks between patches).
    // -------------------------------------------------------------------------
    int numPoints;
    in >> numPoints;

    vector<array<float, 3>> points(numPoints);
    for (int i = 0; i < numPoints; i++) {
        char comma;
        in >> points[i][0] >> comma >> points[i][1] >> comma >> points[i][2];
    }

    // -------------------------------------------------------------------------
    // Step 3: Tessellate each patch into triangles
    // -------------------------------------------------------------------------
    for (const auto& patch : patches) {

        // Build the three 4×4 geometry matrices Gx, Gy, Gz.
        // G[row][col] holds the x (or y or z) coordinate of the control point
        // at position (row, col) in this patch's 4×4 grid.
        // We separate axes so we can apply the same matrix formula to each independently.
        float Gx[4][4], Gy[4][4], Gz[4][4];
        for (int i = 0; i < 16; i++) {
            int idx = patch[i];        // global index of this control point
            int row = i / 4;           // which row in the 4×4 patch grid (0..3)
            int col = i % 4;           // which column in the 4×4 patch grid (0..3)
            Gx[row][col] = points[idx][0];
            Gy[row][col] = points[idx][1];
            Gz[row][col] = points[idx][2];
        }

        // Divide the (u,v) parameter space [0,1]×[0,1] into a uniform grid.
        // Each cell (i,j) covers u in [u0, u1] and v in [v0, v1].
        // We evaluate the surface at the 4 corners of each cell and form 2 triangles:
        //
        //   (u0,v0) ------- (u1,v0)
        //      |          /    |
        //      |        /      |
        //      |      /        |
        //   (u0,v1) ------- (u1,v1)
        //
        //   Triangle 1: (u0,v0) → (u1,v0) → (u1,v1)   [upper-right half]
        //   Triangle 2: (u0,v0) → (u1,v1) → (u0,v1)   [lower-left half]
        //
        for (int i = 0; i < tessellation; i++) {
            float u0 = (float) i      / tessellation;  // left edge of this cell in u
            float u1 = (float)(i + 1) / tessellation;  // right edge of this cell in u

            for (int j = 0; j < tessellation; j++) {
                float v0 = (float) j      / tessellation;  // top edge of this cell in v
                float v1 = (float)(j + 1) / tessellation;  // bottom edge of this cell in v

                // Evaluate the surface at the 4 corners of this cell.
                // Each call to evalBezier computes one 3D point on the surface
                // using the full P(u,v) = U·M·G·M^T·V^T formula.
                float x00, y00, z00; evalBezier(u0, v0, Gx, Gy, Gz, x00, y00, z00); // top-left
                float x10, y10, z10; evalBezier(u1, v0, Gx, Gy, Gz, x10, y10, z10); // top-right
                float x01, y01, z01; evalBezier(u0, v1, Gx, Gy, Gz, x01, y01, z01); // bottom-left
                float x11, y11, z11; evalBezier(u1, v1, Gx, Gy, Gz, x11, y11, z11); // bottom-right

                // Add the two triangles for this cell to the vertex list.
                generateQuad(vertices,
                    x00, y00, z00,   // p1 = top-left
                    x10, y10, z10,   // p2 = top-right
                    x01, y01, z01,   // p3 = bottom-left
                    x11, y11, z11);  // p4 = bottom-right
            }
        }
    }
}