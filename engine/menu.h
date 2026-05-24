#ifndef MENU_H
#define MENU_H

#include "application_state.h"
#include <iostream>
#include <string>

// ============================================================================
// MENU INTERFACE
// ============================================================================

void displayMenu();
void toggleWireframe();
void toggleCulling();
void toggleShowFPS();
void toggleShowAxes();
void toggleShowCurves();
void toggleShowNormals();

#endif // MENU_H
