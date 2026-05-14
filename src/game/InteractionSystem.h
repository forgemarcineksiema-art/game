#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"

#include <cstddef>
#include <string>
#include <vector>

enum class InteractableType {
    Pickup,
    Toggle,
    Info,
};

struct Interactable {
    int id = 0;
    std::string name;
    std::string prompt;
    engine::Vec3 position;
    float radius = 1.5f;
    InteractableType type = InteractableType::Info;
    bool oneShot = false;
    bool enabled = true;
    bool consumed = false;
    bool toggled = false;
    std::string message;
};

struct InteractionFocus {
    bool hasFocus = false;
    int interactableId = 0;
    std::string name;
    std::string prompt;
    engine::Vec3 position;
    float distance = 0.0f;
};

struct InteractionResult {
    bool triggered = false;
    int interactableId = 0;
    std::string name;
    std::string message;
    InteractableType type = InteractableType::Info;
    bool toggled = false;
    bool consumed = false;
};

class InteractionSystem {
public:
    void clear();
    int addInteractable(Interactable interactable);

    InteractionFocus updateFocus(engine::Vec3 playerPosition, engine::Vec3 facingDirection);
    InteractionResult interact(const engine::InputState& input);

    const std::vector<Interactable>& interactables() const;
    const Interactable* interactableById(int id) const;
    const InteractionFocus& focus() const;
    const InteractionResult& lastResult() const;
    std::size_t interactableCount() const;

private:
    Interactable* mutableInteractableById(int id);
    static bool isAvailable(const Interactable& interactable);

    int m_nextId = 1;
    std::vector<Interactable> m_interactables;
    InteractionFocus m_focus;
    InteractionResult m_lastResult;
};
