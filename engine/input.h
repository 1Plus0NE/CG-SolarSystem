#ifndef INPUT_H
#define INPUT_H

#include "application_state.h"
#include "camera_controller.h"
#include "menu.h"

// ============================================================================
// INPUT PROCESSING
// ============================================================================

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
