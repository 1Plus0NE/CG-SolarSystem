#ifndef RENDERING_H
#define RENDERING_H

#include "geometry.h"

// ============================================================================
// RENDERING FLAGS AND STATE
// ============================================================================

extern bool showAxes;
extern bool enableCulling;
extern bool showFPS;
extern bool showEntityCount;
extern int entityCount;
extern float fps;
extern bool wireframeMode;

extern int windowWidth;
extern int windowHeight;
extern Camera camera;
extern bool freeCamera;
extern Group rootGroup;

// ============================================================================
// RENDERING FUNCTIONS
// ============================================================================

/**
 * Generate random stars for skybox
 */
void generateStars();

/**
 * Render the star skybox
 */
void renderStars();

/**
 * Render a group and its children recursively
 */
void renderGroup(const Group& g);

/**
 * Main rendering function (called every frame)
 */
void renderScene(void);

/**
 * Handle window resize
 */
void changeSize(int w, int h);

/**
 * Update FPS counter
 */
void updateFPS();

#endif // RENDERING_H
