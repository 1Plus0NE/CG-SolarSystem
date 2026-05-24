#include "camera_controller.h"
#include "application_state.h"
#include <cmath>

void moveCameraForward(float delta) {
    camera.posX += camera.forwardX * camera.velocity * delta;
    camera.posY += camera.forwardY * camera.velocity * delta;
    camera.posZ += camera.forwardZ * camera.velocity * delta;
}

void moveCameraBackward(float delta) {
    moveCameraForward(-delta);
}

void moveCameraLeft(float delta) {
    camera.posX -= camera.rightX * camera.velocity * delta;
    camera.posY -= camera.rightY * camera.velocity * delta;
    camera.posZ -= camera.rightZ * camera.velocity * delta;
}

void moveCameraRight(float delta) {
    camera.posX += camera.rightX * camera.velocity * delta;
    camera.posY += camera.rightY * camera.velocity * delta;
    camera.posZ += camera.rightZ * camera.velocity * delta;
}

void moveCameraUp(float delta) {
    camera.posY += camera.velocity * delta;
}

void moveCameraDown(float delta) {
    moveCameraUp(-delta);
}

void rotateCameraYaw(float angleDeg) {
    float cosA = cos(angleDeg * M_PI / 180.0f);
    float sinA = sin(angleDeg * M_PI / 180.0f);
    float newFX = camera.forwardX * cosA - camera.forwardZ * sinA;
    float newFZ = camera.forwardX * sinA + camera.forwardZ * cosA;
    camera.forwardX = newFX;
    camera.forwardZ = newFZ;
    float len = sqrt(camera.forwardX*camera.forwardX + camera.forwardY*camera.forwardY + camera.forwardZ*camera.forwardZ);
    camera.forwardX /= len;
    camera.forwardY /= len;
    camera.forwardZ /= len;
}

void rotateCameraPitch(float angleDeg) {
    float cosA = cos(angleDeg * M_PI / 180.0f);
    float sinA = sin(angleDeg * M_PI / 180.0f);
    float newFY = camera.forwardY * cosA - camera.forwardZ * sinA;
    float newFZ = camera.forwardY * sinA + camera.forwardZ * cosA;
    if (newFY >  0.99f) newFY =  0.99f;
    if (newFY < -0.99f) newFY = -0.99f;
    camera.forwardY = newFY;
    camera.forwardZ = newFZ;
    float len = sqrt(camera.forwardX*camera.forwardX + camera.forwardY*camera.forwardY + camera.forwardZ*camera.forwardZ);
    camera.forwardX /= len;
    camera.forwardY /= len;
    camera.forwardZ /= len;
}
