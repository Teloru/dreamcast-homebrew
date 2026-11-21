#include "../sources/scenes/camera_collision.h"

#include <cassert>
#include <cmath>

namespace {

using camera_collision::Box;
using camera_collision::Vec3;
using camera_collision::limit_distance;

void expect_distance(Vec3 origin, Vec3 direction, float desired_distance,
                     const std::vector<Box>& boxes, float expected) {
    const float result = limit_distance(origin, direction, desired_distance, boxes);
    assert(std::isfinite(result));
    assert(std::abs(result - expected) < 0.0001f);
}

} // namespace

int main() {
    const Vec3 origin{0.0f, 0.0f, 0.0f};
    const Vec3 forward{0.0f, 0.0f, 1.0f};
    const Box ahead{{-1.0f, -1.0f, 4.0f}, {1.0f, 1.0f, 6.0f}};

    // No geometry, or geometry beyond the camera, leaves its distance intact.
    expect_distance(origin, forward, 10.0f, {}, 10.0f);
    expect_distance(origin, forward, 3.0f, {ahead}, 3.0f);

    // Cardinal rays hit at distance 4, with 0.1 clearance before the face.
    expect_distance(origin, forward, 10.0f, {ahead}, 3.9f);
    expect_distance(origin, forward, 4.0f, {ahead}, 3.9f);
    expect_distance(origin, {1.0f, 0.0f, 0.0f}, 10.0f,
                    {{{4.0f, -1.0f, -1.0f}, {6.0f, 1.0f, 1.0f}}}, 3.9f);
    expect_distance(origin, {0.0f, -1.0f, 0.0f}, 10.0f,
                    {{{-1.0f, -6.0f, -1.0f}, {1.0f, -4.0f, 1.0f}}}, 3.9f);

    // Parallel rays inside a slab, outside it, or exactly on either face.
    expect_distance({0.5f, 0.0f, 0.0f}, forward, 10.0f, {ahead}, 3.9f);
    expect_distance({2.0f, 0.0f, 0.0f}, forward, 10.0f, {ahead}, 10.0f);
    expect_distance({-1.0f, 0.0f, 0.0f}, forward, 10.0f, {ahead}, 3.9f);
    expect_distance({1.0f, 0.0f, 0.0f}, forward, 10.0f, {ahead}, 3.9f);
    expect_distance({0.0f, -1.0f, 0.0f}, forward, 10.0f, {ahead}, 3.9f);
    expect_distance({0.0f, 1.0f, 0.0f}, forward, 10.0f, {ahead}, 3.9f);
    expect_distance({-1.0f, 0.0f, 0.0f}, {-0.0f, 0.0f, 1.0f},
                    10.0f, {ahead}, 3.9f);
    expect_distance(origin, {0.0000005f, 0.0f, 1.0f}, 10.0f, {ahead}, 3.9f);

    // The nearest obstacle wins regardless of the order in the list.
    const Box farther{{-1.0f, -1.0f, 8.0f}, {1.0f, 1.0f, 9.0f}};
    expect_distance(origin, forward, 10.0f, {farther, ahead}, 3.9f);
    expect_distance(origin, forward, 10.0f, {ahead, farther}, 3.9f);

    // An oblique unit direction enters this box after travelling 4 units.
    expect_distance(origin, {0.6f, 0.0f, 0.8f}, 10.0f,
                    {{{2.4f, -1.0f, 3.2f}, {3.6f, 1.0f, 4.8f}}}, 3.9f);

    const Box behind{{-1.0f, -1.0f, -6.0f}, {1.0f, 1.0f, -4.0f}};
    expect_distance(origin, forward, 10.0f, {behind}, 10.0f);
    expect_distance({0.0f, 0.0f, 5.0f}, forward, 10.0f, {ahead}, 0.1f);

    // Overlapping axis intervals are required, not just individual slab hits.
    expect_distance(origin, {0.6f, 0.0f, 0.8f}, 10.0f, {ahead}, 10.0f);
}
