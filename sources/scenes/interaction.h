#pragma once

namespace interaction {

enum class Target {
    NONE,
    NPC_A,
    NPC_B,
    OBJECT
};

struct Position {
    float x;
    float z;
};

constexpr float RADIUS = 2.2f;

inline Target find_nearby(Position player, Position npc_a, Position npc_b,
                          Position object, bool object_available) {
    const auto within_range = [player](Position target) {
        const float dx = player.x - target.x;
        const float dz = player.z - target.z;
        return dx * dx + dz * dz < RADIUS * RADIUS;
    };

    // Keep the existing priority when interaction areas overlap.
    if(within_range(npc_a)) {
        return Target::NPC_A;
    }
    if(within_range(npc_b)) {
        return Target::NPC_B;
    }
    if(object_available && within_range(object)) {
        return Target::OBJECT;
    }
    return Target::NONE;
}

} // namespace interaction
