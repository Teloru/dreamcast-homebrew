#include "../sources/scenes/interaction.h"

#include <cassert>
#include <iostream>

namespace {

using interaction::Position;
using interaction::Target;
using interaction::find_nearby;

constexpr Position PLAYER{0.0f, 0.0f};
constexpr Position FAR_A{20.0f, 20.0f};
constexpr Position FAR_B{-20.0f, -20.0f};

void test_individual_targets_and_object_availability() {
    assert(find_nearby(PLAYER, FAR_A, FAR_B, FAR_A, true) == Target::NONE);
    assert(find_nearby(PLAYER, {1.0f, 0.0f}, FAR_B, FAR_A, false) == Target::NPC_A);
    assert(find_nearby(PLAYER, FAR_A, {0.0f, -1.0f}, FAR_B, false) == Target::NPC_B);
    assert(find_nearby(PLAYER, FAR_A, FAR_B, {1.0f, 0.0f}, true) == Target::OBJECT);
    assert(find_nearby(PLAYER, FAR_A, FAR_B, {1.0f, 0.0f}, false) == Target::NONE);
}

void test_priority_is_preserved_in_overlapping_areas() {
    // NPC A wins even when NPC B and the object are closer.
    assert(find_nearby(PLAYER, {2.0f, 0.0f}, {0.1f, 0.0f}, PLAYER, true) ==
           Target::NPC_A);
    assert(find_nearby(PLAYER, FAR_A, {2.0f, 0.0f}, PLAYER, true) == Target::NPC_B);
    assert(find_nearby(PLAYER, PLAYER, FAR_B, PLAYER, false) == Target::NPC_A);
}

void test_radius_excludes_the_exact_boundary() {
    const Position inside{interaction::RADIUS - 0.001f, 0.0f};
    const Position boundary{interaction::RADIUS, 0.0f};
    const Position outside{interaction::RADIUS + 0.001f, 0.0f};

    assert(find_nearby(PLAYER, inside, FAR_B, FAR_A, false) == Target::NPC_A);
    assert(find_nearby(PLAYER, boundary, FAR_B, FAR_A, false) == Target::NONE);
    assert(find_nearby(PLAYER, outside, FAR_B, FAR_A, false) == Target::NONE);

    assert(find_nearby(PLAYER, FAR_A, inside, FAR_B, false) == Target::NPC_B);
    assert(find_nearby(PLAYER, FAR_A, boundary, FAR_B, false) == Target::NONE);
    assert(find_nearby(PLAYER, FAR_A, outside, FAR_B, false) == Target::NONE);

    assert(find_nearby(PLAYER, FAR_A, FAR_B, inside, true) == Target::OBJECT);
    assert(find_nearby(PLAYER, FAR_A, FAR_B, boundary, true) == Target::NONE);
    assert(find_nearby(PLAYER, FAR_A, FAR_B, outside, true) == Target::NONE);
}

void test_distance_uses_both_coordinates() {
    const float diagonal_inside = interaction::RADIUS * 0.7f;
    const float diagonal_outside = interaction::RADIUS * 0.8f;
    assert(find_nearby(PLAYER, {diagonal_inside, diagonal_inside}, FAR_B,
                       FAR_A, false) == Target::NPC_A);
    assert(find_nearby(PLAYER, {diagonal_outside, diagonal_outside}, FAR_B,
                       FAR_A, false) == Target::NONE);
    assert(find_nearby({-13.0f, -12.0f}, {-12.0f, -12.0f}, FAR_B,
                       FAR_A, false) == Target::NPC_A);
}

void test_moving_npcs_changes_their_interaction_areas() {
    Position npc_a{-12.0f, -12.0f};
    const Position player{-13.0f, -12.0f};
    assert(find_nearby(player, npc_a, FAR_B, FAR_A, false) == Target::NPC_A);

    npc_a = {8.0f, 8.0f};
    assert(find_nearby(player, npc_a, FAR_B, FAR_A, false) == Target::NONE);
    assert(find_nearby({8.0f, 8.0f}, npc_a, FAR_B, FAR_A, false) == Target::NPC_A);

    Position npc_b{1.0f, 0.0f};
    assert(find_nearby(PLAYER, FAR_A, npc_b, FAR_B, false) == Target::NPC_B);
    npc_b = {8.0f, -8.0f};
    assert(find_nearby(PLAYER, FAR_A, npc_b, FAR_B, false) == Target::NONE);
    assert(find_nearby({8.0f, -8.0f}, FAR_A, npc_b, FAR_B, false) == Target::NPC_B);
}

} // namespace

int main() {
    test_individual_targets_and_object_availability();
    test_priority_is_preserved_in_overlapping_areas();
    test_radius_excludes_the_exact_boundary();
    test_distance_uses_both_coordinates();
    test_moving_npcs_changes_their_interaction_areas();
    std::cout << "interaction tests passed\n";
}
