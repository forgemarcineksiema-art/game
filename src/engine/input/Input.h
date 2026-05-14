#pragma once

namespace engine {

struct InputState {
    bool quitRequested = false;
    float moveRight = 0.0f;
    float moveForward = 0.0f;
    bool sprintHeld = false;
    bool jumpPressed = false;
    float cameraYawDelta = 0.0f;
    float cameraPitchDelta = 0.0f;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
};

} // namespace engine
