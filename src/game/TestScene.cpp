#include "game/TestScene.h"

TestScene::TestScene()
{
    m_world.buildDefaultCollisionTestLayout();

    Interactable pickup;
    pickup.name = "Test Pickup";
    pickup.prompt = "Pick up Test Item";
    pickup.position = {0.0f, 0.45f, 1.35f};
    pickup.radius = 1.8f;
    pickup.type = InteractableType::Pickup;
    pickup.oneShot = true;
    pickup.message = "Picked up Test Item.";
    m_interactions.addInteractable(pickup);

    Interactable toggle;
    toggle.name = "Wall Button";
    toggle.prompt = "Toggle Test Door";
    toggle.position = {-0.9f, 0.7f, 2.15f};
    toggle.radius = 1.7f;
    toggle.type = InteractableType::Toggle;
    m_interactions.addInteractable(toggle);

    Interactable info;
    info.name = "Info Marker";
    info.prompt = "Read Debug Marker";
    info.position = {2.2f, 0.55f, -0.8f};
    info.radius = 1.7f;
    info.type = InteractableType::Info;
    info.message = "Debug marker: interaction plumbing online.";
    m_interactions.addInteractable(info);

    Interactable maintenanceBox;
    maintenanceBox.name = "Maintenance Box";
    maintenanceBox.prompt = "Inspect Maintenance Box";
    maintenanceBox.position = {2.8f, 0.65f, 0.25f};
    maintenanceBox.radius = 1.6f;
    maintenanceBox.type = InteractableType::Info;
    maintenanceBox.message = "Maintenance box placeholder: reachable after traversal.";
    m_interactions.addInteractable(maintenanceBox);

    TraversalAffordance serviceVault;
    serviceVault.name = "Service Barrier Vault";
    serviceVault.prompt = "Press Space: Vault Service Barrier";
    serviceVault.type = TraversalType::Vault;
    serviceVault.startPosition = {2.8f, 0.0f, -2.35f};
    serviceVault.endPosition = {2.8f, 0.0f, -0.55f};
    serviceVault.focusRadius = 1.45f;
    serviceVault.requiredFacingDirection = {0.0f, 0.0f, 1.0f};
    serviceVault.requiredFacingDot = 0.15f;
    serviceVault.durationSeconds = 0.55f;
    m_traversal.addAffordance(serviceVault);
}

TestWorld& TestScene::world()
{
    return m_world;
}

const TestWorld& TestScene::world() const
{
    return m_world;
}

InteractionSystem& TestScene::interactions()
{
    return m_interactions;
}

const InteractionSystem& TestScene::interactions() const
{
    return m_interactions;
}

TraversalSystem& TestScene::traversal()
{
    return m_traversal;
}

const TraversalSystem& TestScene::traversal() const
{
    return m_traversal;
}
