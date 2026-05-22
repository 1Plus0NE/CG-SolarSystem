#include "application_state.h"

bool  showAxes       = false;
bool  enableCulling  = true;
bool  showFPS        = false;
bool  showEntityCount= false;
bool  showCurves     = false;
int   entityCount    = 0;
float fps            = 0.0f;
bool  wireframeMode  = false;
float frameTime      = 0.0f;

int windowWidth  = 800;
int windowHeight = 600;

Camera camera;
bool   freeCamera = false;

Group              rootGroup;
std::vector<Light> sceneLights;

bool enableFrameLog    = false;
bool disableVBO        = false;
int  frameLogMaxRecords= -1;
