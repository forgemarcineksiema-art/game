#include "game/InteractionSystem.h"

#include <limits>
#include <sstream>
#include <utility>

namespace {

constexpr float CloseFocusDistance = 0.75f;
constexpr float MinFacingDot = 0.25f;

engine::Vec3 Flatten(engine::Vec3 value)
{
    value.y = 0.0f;
    return value;
}

std::string DefaultMessage(const Interactable& interactable)
{
    if (!interactable.message.empty()) {
        return interactable.message;
    }

    switch (interactable.type) {
    case InteractableType::Pickup:
        return "Picked up " + interactable.name + ".";
    case InteractableType::Toggle: {
        std::ostringstream output;
        output << interactable.name << (interactable.toggled ? " toggled on." : " toggled off.");
        return output.str();
    }
    case InteractableType::Info:
    default:
        return interactable.name + " inspected.";
    }
}

} // namespace

void InteractionSystem::clear()
{
    m_interactables.clear();
    m_nextId = 1;
    m_focus = {};
    m_lastResult = {};
}

int InteractionSystem::addInteractable(Interactable interactable)
{
    interactable.id = m_nextId++;
    m_interactables.push_back(std::move(interactable));
    return m_interactables.back().id;
}

InteractionFocus InteractionSystem::updateFocus(engine::Vec3 playerPosition, engine::Vec3 facingDirection)
{
    InteractionFocus bestFocus;
    float bestScore = std::numeric_limits<float>::max();
    const engine::Vec3 flatFacing = engine::Normalize(Flatten(facingDirection));
    const bool hasFacing = engine::Length(flatFacing) > 0.00001f;

    for (const Interactable& interactable : m_interactables) {
        if (!isAvailable(interactable)) {
            continue;
        }

        const engine::Vec3 toInteractable = Flatten(interactable.position - playerPosition);
        const float distance = engine::Length(toInteractable);
        if (distance > interactable.radius) {
            continue;
        }

        float facingDot = 1.0f;
        if (distance > 0.00001f && hasFacing) {
            facingDot = engine::Dot(engine::Normalize(toInteractable), flatFacing);
        }

        if (distance > CloseFocusDistance && facingDot < MinFacingDot) {
            continue;
        }

        const float score = distance - facingDot * 0.1f;
        if (score < bestScore) {
            bestScore = score;
            bestFocus.hasFocus = true;
            bestFocus.interactableId = interactable.id;
            bestFocus.name = interactable.name;
            bestFocus.prompt = interactable.prompt;
            bestFocus.position = interactable.position;
            bestFocus.distance = distance;
        }
    }

    m_focus = bestFocus;
    return m_focus;
}

InteractionResult InteractionSystem::interact(const engine::InputState& input)
{
    m_lastResult = {};
    if (!input.interactPressed || !m_focus.hasFocus) {
        return m_lastResult;
    }

    Interactable* interactable = mutableInteractableById(m_focus.interactableId);
    if (!interactable || !isAvailable(*interactable)) {
        m_focus = {};
        return m_lastResult;
    }

    switch (interactable->type) {
    case InteractableType::Pickup:
        interactable->consumed = true;
        interactable->enabled = false;
        break;
    case InteractableType::Toggle:
        interactable->toggled = !interactable->toggled;
        break;
    case InteractableType::Info:
    default:
        break;
    }

    m_lastResult.triggered = true;
    m_lastResult.interactableId = interactable->id;
    m_lastResult.name = interactable->name;
    m_lastResult.type = interactable->type;
    m_lastResult.toggled = interactable->toggled;
    m_lastResult.consumed = interactable->consumed;
    m_lastResult.message = DefaultMessage(*interactable);

    if (interactable->oneShot && interactable->consumed) {
        m_focus = {};
    }

    return m_lastResult;
}

const std::vector<Interactable>& InteractionSystem::interactables() const
{
    return m_interactables;
}

const Interactable* InteractionSystem::interactableById(int id) const
{
    for (const Interactable& interactable : m_interactables) {
        if (interactable.id == id) {
            return &interactable;
        }
    }

    return nullptr;
}

const InteractionFocus& InteractionSystem::focus() const
{
    return m_focus;
}

const InteractionResult& InteractionSystem::lastResult() const
{
    return m_lastResult;
}

std::size_t InteractionSystem::interactableCount() const
{
    return m_interactables.size();
}

Interactable* InteractionSystem::mutableInteractableById(int id)
{
    for (Interactable& interactable : m_interactables) {
        if (interactable.id == id) {
            return &interactable;
        }
    }

    return nullptr;
}

bool InteractionSystem::isAvailable(const Interactable& interactable)
{
    return interactable.enabled && !interactable.consumed;
}
