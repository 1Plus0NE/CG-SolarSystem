#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

// Free-fly camera movement along the forward/right vectors.
void moveCameraForward(float delta);
void moveCameraBackward(float delta);
void moveCameraLeft(float delta);
void moveCameraRight(float delta);

// Yaw rotates around world-up; pitch rotates around the right axis (clamped).
void rotateCameraYaw(float angleDeg);
void rotateCameraPitch(float angleDeg);

#endif // CAMERA_CONTROLLER_H
