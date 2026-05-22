#ifndef RENDERING_H
#define RENDERING_H

#include "application_state.h"
#include <string>

// ============================================================================
// RENDERING STATE (defined in rendering.cpp)
// ============================================================================

extern std::string frameLogOutputFile;

// ============================================================================
// RENDERING FUNCTIONS
// ============================================================================

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
 * Update FPS counter and collect frame-time samples
 */
void updateFPS();

/**
 * Flush collected frame-time samples to disk
 */
void flushFrameLog();

#endif // RENDERING_H
