#pragma once

#include "engine/math/Math.h"

#include <cstddef>
#include <string>

struct NeutralSceneRuntimeFocus {
    bool hasFocus = false;
    std::string name = "none";
    std::string prompt;
};

struct NeutralSceneRuntimeView {
    bool sceneDefinitionLoaded = false;
    std::string sceneId = "built-in-fallback";
    std::string sceneName = "Built-in fallback";
    std::string sceneRole = "fallback";
    std::string worldId = "fallback";
    std::string sliceId = "fallback";
    std::string objectiveText;
    std::string debugObjectiveText;
    std::string completionSummary;
    bool sliceComplete = false;
    NeutralSceneRuntimeFocus interactionFocus;
    NeutralSceneRuntimeFocus traversalFocus;
    bool traversalPromptActive = false;
    engine::Vec3 playerPosition {};
    float playerYawRadians = 0.0f;
    float playerHorizontalSpeed = 0.0f;
    bool playerSprinting = false;
    bool playerGrounded = true;
    int playerCollisionHits = 0;
    float cameraYawRadians = 0.0f;
    float cameraPitchRadians = 0.0f;
    float cameraDistance = 0.0f;
    std::size_t colliderCount = 0;
    std::size_t interactableCount = 0;
    std::size_t routeCount = 0;
    std::size_t markerCount = 0;
    bool vehicleAvailable = false;
    std::string physicsText = "none";
    std::string vehicleRuntimeText = "none";
    bool interactPressed = false;
    bool traversalPressed = false;
    std::string lastInteractionText = "none";
    std::string lastWorldEventText = "none";
};

std::string BuildNeutralScenePresentationText(const NeutralSceneRuntimeView& view, bool minimal);
std::string BuildNeutralSceneDebugText(const NeutralSceneRuntimeView& view);
