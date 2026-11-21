#include "../sources/scenes/walking.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

bool near(float actual, float expected) {
    return std::abs(actual - expected) < 0.0001f;
}

void test_free_diagonal_movement() {
    const auto position = walking::move({1.0f, 2.0f}, 2.0f, -3.0f, {});
    assert(near(position.x, 3.0f));
    assert(near(position.z, -1.0f));

    const auto stationary = walking::move(position, 0.0f, 0.0f, {});
    assert(stationary.x == position.x);
    assert(stationary.z == position.z);
}

void test_frontal_blocking_and_sliding() {
    const std::vector<walking::Obstacle> obstacles = {
        {0.0f, 0.0f, 0.5f, 0.5f}
    };
    const float contact_x = -(0.5f + walking::PLAYER_RADIUS);

    const auto blocked = walking::move({-2.0f, 0.0f}, 4.0f, 0.0f, obstacles);
    assert(walking::can_stand_at(blocked, obstacles));
    assert(blocked.x <= contact_x);
    assert(blocked.x >= contact_x - 0.1001f);
    assert(near(blocked.z, 0.0f));

    const auto sliding = walking::move({-1.0f, 0.0f}, 1.0f, 0.5f, obstacles);
    assert(walking::can_stand_at(sliding, obstacles));
    assert(sliding.x <= contact_x);
    assert(sliding.x > -1.0f);
    assert(near(sliding.z, 0.5f));
}

void test_large_step_cannot_cross_thin_obstacle() {
    const std::vector<walking::Obstacle> obstacles = {
        {0.0f, 0.0f, 0.01f, 3.0f}
    };
    const float contact_x = -(0.01f + walking::PLAYER_RADIUS);
    const auto position = walking::move({-2.0f, 0.0f}, 8.0f, 0.0f, obstacles);
    assert(walking::can_stand_at(position, obstacles));
    assert(position.x <= contact_x);
    assert(position.x >= contact_x - 0.1001f);
    assert(near(position.z, 0.0f));
}

void test_world_limits() {
    const float limit = walking::WORLD_HALF_SIZE - walking::PLAYER_RADIUS;
    assert(walking::can_stand_at({limit, -limit}, {}));
    assert(!walking::can_stand_at({limit + 0.01f, 0.0f}, {}));
    assert(!walking::can_stand_at({-limit - 0.01f, 0.0f}, {}));
    assert(!walking::can_stand_at({0.0f, limit + 0.01f}, {}));
    assert(!walking::can_stand_at({0.0f, -limit - 0.01f}, {}));

    const auto position = walking::move({0.0f, 0.0f}, 30.0f, -30.0f, {});
    assert(walking::can_stand_at(position, {}));
    assert(position.x <= limit);
    assert(position.x >= limit - 0.1001f);
    assert(position.z >= -limit);
    assert(position.z <= -limit + 0.1001f);
}

void test_tangency_is_allowed() {
    const std::vector<walking::Obstacle> obstacles = {
        {0.0f, 0.0f, 0.5f, 0.5f}
    };
    const float contact = 0.5f + walking::PLAYER_RADIUS;
    assert(walking::can_stand_at({contact, 0.0f}, obstacles));
    assert(walking::can_stand_at({0.0f, -contact}, obstacles));
    assert(!walking::can_stand_at({contact - 0.001f, 0.0f}, obstacles));

    const auto position = walking::move({-contact, -0.5f}, 0.0f, 1.0f,
                                        obstacles);
    assert(near(position.x, -contact));
    assert(near(position.z, 0.5f));
}

void test_equivalent_time_partitions_without_obstacles() {
    const walking::Position start = {0.0f, 11.0f};
    assert(walking::can_stand_at(start, {}));

    const auto whole = walking::move(start, 3.0f, -2.0f, {});
    auto partitioned = start;
    for(int i = 0; i < 30; ++i) {
        partitioned = walking::move(partitioned, 3.0f / 30, -2.0f / 30, {});
    }
    assert(near(partitioned.x, whole.x));
    assert(near(partitioned.z, whole.z));

    const auto returned = walking::move(whole, -3.0f, 2.0f, {});
    assert(near(returned.x, start.x));
    assert(near(returned.z, start.z));
}

} // namespace

int main() {
    test_free_diagonal_movement();
    test_frontal_blocking_and_sliding();
    test_large_step_cannot_cross_thin_obstacle();
    test_world_limits();
    test_tangency_is_allowed();
    test_equivalent_time_partitions_without_obstacles();
    std::cout << "walking tests passed\n";
}
