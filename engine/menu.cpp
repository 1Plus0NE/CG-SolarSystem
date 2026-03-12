#include "menu.h"

// ============================================================================
// MENU FUNCTIONS
// ============================================================================

void displayMenu() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║      SOLARIUM - MENU DE OPÇÕES         ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║                                        ║\n";
    std::cout << "║  W - Wireframe: "
              << (wireframeMode ? "✓ ON " : "✗ OFF") << "                  ║\n";
    std::cout << "║  C - Culling:   "
              << (enableCulling ? "✓ ON " : "✗ OFF") << "                  ║\n";
    std::cout << "║  O - Show FPS:  "
              << (showFPS ? "✓ ON " : "✗ OFF") << "                  ║\n";
    std::cout << "║  A - Show Axes: "
              << (showAxes ? "✓ ON " : "✗ OFF") << "                  ║\n";
    std::cout << "║                                        ║\n";
    std::cout << "║  CAMERA CONTROLS:                     ║\n";
    std::cout << "║  I/K - Rotate vertical                ║\n";
    std::cout << "║  J/L - Rotate horizontal              ║\n";
    std::cout << "║  +/- - Zoom in/out                    ║\n";
    std::cout << "║  R   - Reload config                  ║\n";
    std::cout << "║  M   - Show this menu                 ║\n";
    std::cout << "║  ESC - Exit                           ║\n";
    std::cout << "║                                        ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void toggleWireframe() {
    wireframeMode = !wireframeMode;
    std::cout << "→ Wireframe: " << (wireframeMode ? "ON ✓" : "OFF ✗") << std::endl;
}

void toggleCulling() {
    enableCulling = !enableCulling;
    std::cout << "→ Culling: " << (enableCulling ? "ON ✓" : "OFF ✗") << std::endl;
}

void toggleShowFPS() {
    showFPS = !showFPS;
    std::cout << "→ Show FPS: " << (showFPS ? "ON ✓" : "OFF ✗") << std::endl;
}

void toggleShowAxes() {
    showAxes = !showAxes;
    std::cout << "→ Show Axes: " << (showAxes ? "ON ✓" : "OFF ✗") << std::endl;
}
