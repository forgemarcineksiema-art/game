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
