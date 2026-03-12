#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>

// ============================================================================
// MENU INTERFACE
// ============================================================================

extern bool wireframeMode;
extern bool enableCulling;
extern bool showFPS;
extern bool showAxes;

/**
 * Display the menu with available options
 */
void displayMenu();

/**
 * Toggle wireframe mode
 */
void toggleWireframe();

/**
 * Toggle face culling
 */
void toggleCulling();

/**
 * Toggle FPS display
 */
void toggleShowFPS();

/**
 * Toggle axes display
 */
void toggleShowAxes();

#endif // MENU_H
