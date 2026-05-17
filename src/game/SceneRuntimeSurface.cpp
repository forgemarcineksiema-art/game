#include "game/SceneRuntimeSurface.h"

#include <iomanip>
#include <sstream>

std::string BuildNeutralScenePresentationText(const NeutralSceneRuntimeView& view, bool minimal)
{
    const std::string vehicleText = view.vehicleAvailable ? "authored" : "none";

    std::ostringstream output;
    output << "Scene: " << view.sceneName << " | role=" << view.sceneRole << "\n"
           << "Objective: " << view.objectiveText << "\n";

    bool hasPrompt = false;
    if (view.interactionFocus.hasFocus) {
        output << "Prompt: Press E: " << view.interactionFocus.prompt << "\n";
        hasPrompt = true;
    } else if (view.traversalFocus.hasFocus && view.traversalPromptActive) {
        output << "Prompt: Press Space: " << view.traversalFocus.prompt << "\n";
        hasPrompt = true;
    }
    if (!hasPrompt && !minimal) {
        output << "Prompt: Inspect authored neutral markers; no regression job chain is active.\n";
    }

    if (!minimal) {
        output << "Status: colliders=" << view.colliderCount
               << " | interactables=" << view.interactableCount
               << " | routes=" << view.routeCount
               << " | markers=" << view.markerCount
               << " | vehicle=" << vehicleText << "\n"
               << "F1: debug | Esc: quit";
    }

    return output.str();
}

std::string BuildNeutralSceneDebugText(const NeutralSceneRuntimeView& view)
{
    const std::string vehicleText = view.vehicleAvailable ? "authored" : "none";

    std::ostringstream output;
    output << std::fixed << std::setprecision(2)
           << "scene=" << view.sceneId << " loaded=" << (view.sceneDefinitionLoaded ? "yes" : "no")
           << " role=" << view.sceneRole << " world=" << view.worldId << " sliceId=" << view.sliceId << "\n"
           << "objective=\"" << (view.debugObjectiveText.empty() ? view.objectiveText : view.debugObjectiveText) << "\" "
           << "sliceComplete=" << (view.sliceComplete ? "yes" : "no") << "\n"
           << "focus=" << (view.interactionFocus.hasFocus ? view.interactionFocus.name : "none") << " ";
    if (view.interactionFocus.hasFocus) {
        output << "prompt=\"Press E: " << view.interactionFocus.prompt << "\" ";
    }
    if (view.traversalFocus.hasFocus && view.traversalPromptActive) {
        output << "travPrompt=\"" << view.traversalFocus.prompt << "\" ";
    }
    output << "travFocus=" << (view.traversalFocus.hasFocus ? view.traversalFocus.name : "none") << "\n"
           << "player=(" << view.playerPosition.x << "," << view.playerPosition.y << "," << view.playerPosition.z << ") "
           << "playerYaw=" << engine::Degrees(view.playerYawRadians) << " "
           << "speed=" << view.playerHorizontalSpeed << " "
           << (view.playerSprinting ? "sprint" : "walk") << " "
           << (view.playerGrounded ? "grounded" : "air") << " "
           << "hits=" << view.playerCollisionHits << " "
           << "cameraMode=on-foot "
           << "camera yaw=" << engine::Degrees(view.cameraYawRadians)
           << " pitch=" << engine::Degrees(view.cameraPitchRadians)
           << " dist=" << view.cameraDistance << "\n"
           << "sceneCounts=colliders:" << view.colliderCount
           << " interactables:" << view.interactableCount
           << " routes:" << view.routeCount
           << " markers:" << view.markerCount
           << " vehicle:" << vehicleText << " "
           << "physics=" << view.physicsText << " "
           << "vehicleRuntime=" << view.vehicleRuntimeText << "\n"
           << "input=interact:" << (view.interactPressed ? "yes" : "no")
           << " traversal:" << (view.traversalPressed ? "yes" : "no") << " "
           << "lastInteraction=\"" << view.lastInteractionText << "\" "
           << "lastWorldEvent=\"" << view.lastWorldEventText << "\"\n"
           << "slice={" << view.completionSummary << "}";
    return output.str();
}
