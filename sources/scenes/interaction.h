#pragma once

#include <cmath>

namespace interaction {

enum class Target {
    NONE,
    NPC_A,
    NPC_B,
    OBJECT
};

struct Position {
    float x;
    float y;
    float z;
};

constexpr float RADIUS = 2.2f;
// Allow small differences in footing, but not a full stair step (0.75).
constexpr float PICKUP_HEIGHT_TOLERANCE = 0.25f;

// The player's position is at their feet; the object's position is at its base.
inline Target find_nearby(Position player, Position npc_a, Position npc_b,
                          Position object, bool object_available) {
    const auto within_horizontal_range = [player](Position target) {
        const float dx = player.x - target.x;
        const float dz = player.z - target.z;
        return dx * dx + dz * dz < RADIUS * RADIUS;
    };

    // Keep the existing priority when interaction areas overlap.
    if(within_horizontal_range(npc_a)) {
        return Target::NPC_A;
    }
    if(within_horizontal_range(npc_b)) {
        return Target::NPC_B;
    }
    if(object_available && within_horizontal_range(object) &&
       std::abs(player.y - object.y) < PICKUP_HEIGHT_TOLERANCE) {
        return Target::OBJECT;
    }
    return Target::NONE;
}

} // namespace interaction
