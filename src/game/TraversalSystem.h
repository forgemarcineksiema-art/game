#pragma once

#include "engine/input/Input.h"
#include "engine/math/Math.h"

#include <cstddef>
#include <string>
#include <vector>

enum class TraversalType {
    Vault,
};

struct TraversalAffordance {
    int id = 0;
    std::string name;
    TraversalType type = TraversalType::Vault;
    engine::Vec3 startPosition;
    engine::Vec3 endPosition;
    float focusRadius = 1.5f;
    engine::Vec3 requiredFacingDirection = {0.0f, 0.0f, 1.0f};
    float requiredFacingDot = 0.25f;
    bool enabled = true;
    std::string prompt;
    float durationSeconds = 0.45f;
};

struct TraversalFocus {
    bool hasFocus = false;
    int affordanceId = 0;
    std::string name;
    std::string prompt;
    TraversalType type = TraversalType::Vault;
    engine::Vec3 startPosition;
    engine::Vec3 endPosition;
    float distance = 0.0f;
    float durationSeconds = 0.0f;
};

struct TraversalActivation {
    bool started = false;
    int affordanceId = 0;
    std::string name;
    TraversalType type = TraversalType::Vault;
    engine::Vec3 startPosition;
    engine::Vec3 endPosition;
    float durationSeconds = 0.0f;
};

class TraversalSystem {
public:
    void clear();
    int addAffordance(TraversalAffordance affordance);

    TraversalFocus updateFocus(engine::Vec3 playerPosition, engine::Vec3 facingDirection);
    TraversalActivation activationFromInput(const engine::InputState& input) const;

    const std::vector<TraversalAffordance>& affordances() const;
    const TraversalAffordance* affordanceById(int id) const;
    const TraversalFocus& focus() const;
    std::size_t affordanceCount() const;

private:
    static bool isAvailable(const TraversalAffordance& affordance);

    int m_nextId = 1;
    std::vector<TraversalAffordance> m_affordances;
    TraversalFocus m_focus;
};
