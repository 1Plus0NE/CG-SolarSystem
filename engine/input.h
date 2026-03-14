#ifndef INPUT_H
#define INPUT_H

#include "geometry.h"
#include "menu.h"

// ============================================================================
// INPUT PROCESSING
// ============================================================================

extern Camera camera;
extern bool wireframeMode;
extern bool enableCulling;
extern bool showFPS;
extern bool showAxes;
extern bool freeCamera;

/**
 * Handle keyboard input
 */
void processKeys(unsigned char c, int xx, int yy);

/**
 * Handle mouse button input
 */
void processMouseButtons(int button, int state, int x, int y);

/**
 * Handle mouse motion input
 */
void processMouseMotion(int x, int y);

#endif // INPUT_H
