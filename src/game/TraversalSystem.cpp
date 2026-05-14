#include "game/TraversalSystem.h"

#include <limits>
#include <utility>

namespace {

constexpr float CloseTraversalFocusDistance = 0.75f;

engine::Vec3 Flatten(engine::Vec3 value)
{
    value.y = 0.0f;
    return value;
}

std::string DefaultPrompt(const TraversalAffordance& affordance)
{
    if (!affordance.prompt.empty()) {
        return affordance.prompt;
    }

    return "Press Space: Vault " + affordance.name;
}

} // namespace

void TraversalSystem::clear()
{
    m_affordances.clear();
    m_nextId = 1;
    m_focus = {};
}

int TraversalSystem::addAffordance(TraversalAffordance affordance)
{
    affordance.id = m_nextId++;
    m_affordances.push_back(std::move(affordance));
    return m_affordances.back().id;
}

TraversalFocus TraversalSystem::updateFocus(engine::Vec3 playerPosition, engine::Vec3 facingDirection)
{
    TraversalFocus bestFocus;
    float bestScore = std::numeric_limits<float>::max();
    const engine::Vec3 flatFacing = engine::Normalize(Flatten(facingDirection));
    const bool hasFacing = engine::Length(flatFacing) > 0.00001f;

    for (const TraversalAffordance& affordance : m_affordances) {
        if (!isAvailable(affordance)) {
            continue;
        }

        const engine::Vec3 toStart = Flatten(affordance.startPosition - playerPosition);
        const float distance = engine::Length(toStart);
        if (distance > affordance.focusRadius) {
            continue;
        }

        float facingDot = 1.0f;
        const engine::Vec3 requiredFacing = engine::Normalize(Flatten(affordance.requiredFacingDirection));
        if (hasFacing && engine::Length(requiredFacing) > 0.00001f) {
            facingDot = engine::Dot(flatFacing, requiredFacing);
        }

        if (distance > CloseTraversalFocusDistance && facingDot < affordance.requiredFacingDot) {
            continue;
        }

        const float score = distance - facingDot * 0.1f;
        if (score < bestScore) {
            bestScore = score;
            bestFocus.hasFocus = true;
            bestFocus.affordanceId = affordance.id;
            bestFocus.name = affordance.name;
            bestFocus.prompt = DefaultPrompt(affordance);
            bestFocus.type = affordance.type;
            bestFocus.startPosition = affordance.startPosition;
            bestFocus.endPosition = affordance.endPosition;
            bestFocus.distance = distance;
            bestFocus.durationSeconds = affordance.durationSeconds;
        }
    }

    m_focus = bestFocus;
    return m_focus;
}

TraversalActivation TraversalSystem::activationFromInput(const engine::InputState& input) const
{
    TraversalActivation activation;
    if (!input.jumpPressed || !m_focus.hasFocus) {
        return activation;
    }

    activation.started = true;
    activation.affordanceId = m_focus.affordanceId;
    activation.name = m_focus.name;
    activation.type = m_focus.type;
    activation.startPosition = m_focus.startPosition;
    activation.endPosition = m_focus.endPosition;
    activation.durationSeconds = m_focus.durationSeconds;
    return activation;
}

const std::vector<TraversalAffordance>& TraversalSystem::affordances() const
{
    return m_affordances;
}

const TraversalAffordance* TraversalSystem::affordanceById(int id) const
{
    for (const TraversalAffordance& affordance : m_affordances) {
        if (affordance.id == id) {
            return &affordance;
        }
    }

    return nullptr;
}

const TraversalFocus& TraversalSystem::focus() const
{
    return m_focus;
}

std::size_t TraversalSystem::affordanceCount() const
{
    return m_affordances.size();
}

bool TraversalSystem::isAvailable(const TraversalAffordance& affordance)
{
    return affordance.enabled;
}
