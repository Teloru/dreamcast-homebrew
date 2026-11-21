#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace walking {

constexpr float WORLD_HALF_SIZE = 16.0f;
constexpr float PLAYER_RADIUS = 0.4f;

struct Position {
    float x;
    float z;
};

struct Obstacle {
    float x;
    float z;
    float half_width;
    float half_depth;
};

inline bool can_stand_at(Position position,
                         const std::vector<Obstacle>& obstacles) {
    const float limit = WORLD_HALF_SIZE - PLAYER_RADIUS;
    if(std::abs(position.x) > limit || std::abs(position.z) > limit) {
        return false;
    }

    for(const auto& obstacle : obstacles) {
        if(std::abs(position.x - obstacle.x) <
               obstacle.half_width + PLAYER_RADIUS &&
           std::abs(position.z - obstacle.z) <
               obstacle.half_depth + PLAYER_RADIUS) {
            return false;
        }
    }
    return true;
}

inline Position move(Position start, float dx, float dz,
                     const std::vector<Obstacle>& obstacles) {
    const int steps = static_cast<int>(
        std::ceil(std::max(std::abs(dx), std::abs(dz)) / 0.1f));
    if(steps == 0) {
        return start;
    }

    const float step_x = dx / steps;
    const float step_z = dz / steps;
    Position position = start;
    for(int i = 0; i < steps; ++i) {
        const Position next_x = {position.x + step_x, position.z};
        if(can_stand_at(next_x, obstacles)) {
            position.x = next_x.x;
        }

        const Position next_z = {position.x, position.z + step_z};
        if(can_stand_at(next_z, obstacles)) {
            position.z = next_z.z;
        }
    }
    return position;
}

} // namespace walking
