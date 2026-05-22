#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include "geometry.h"
#include <vector>

// Single source of truth for all engine-wide global state.
// Definitions live in application_state.cpp; every other module uses these externs.

// Rendering toggles
extern bool  showAxes;
extern bool  enableCulling;
extern bool  showFPS;
extern bool  showEntityCount;
extern bool  showCurves;
extern int   entityCount;
extern float fps;
extern bool  wireframeMode;
extern float frameTime;

// Window
extern int windowWidth;
extern int windowHeight;

// Camera
extern Camera camera;
extern bool   freeCamera;

// Scene graph
extern Group              rootGroup;
extern std::vector<Light> sceneLights;

// Frame-logging options (values assigned in main())
extern bool enableFrameLog;
extern bool disableVBO;
extern int  frameLogMaxRecords;

#endif // APPLICATION_STATE_H
