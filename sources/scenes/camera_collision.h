#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace camera_collision {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Box {
    Vec3 min;
    Vec3 max;
};

// Direction is a unit vector. Boxes already include the camera's clearance.
inline float limit_distance(Vec3 origin, Vec3 direction, float desired_distance,
                            const std::vector<Box>& boxes) {
    const float start[] = {origin.x, origin.y, origin.z};
    const float delta[] = {direction.x, direction.y, direction.z};
    float nearest = desired_distance;
    bool found_hit = false;

    for(const auto& box : boxes) {
        const float lower[] = {box.min.x, box.min.y, box.min.z};
        const float upper[] = {box.max.x, box.max.y, box.max.z};
        float enter = 0.0f;
        float exit = desired_distance;
        bool intersects = true;

        for(int axis = 0; axis < 3; ++axis) {
            // A parallel ray stays inside or outside this slab throughout.
            // Avoid division by zero, including when it lies on a box face.
            if(std::abs(delta[axis]) < 0.000001f) {
                if(start[axis] < lower[axis] || start[axis] > upper[axis]) {
                    intersects = false;
                    break;
                }
                continue;
            }

            float near = (lower[axis] - start[axis]) / delta[axis];
            float far = (upper[axis] - start[axis]) / delta[axis];
            if(near > far) {
                std::swap(near, far);
            }
            enter = std::max(enter, near);
            exit = std::min(exit, far);
            if(enter > exit) {
                intersects = false;
                break;
            }
        }

        if(intersects) {
            nearest = std::min(nearest, enter);
            found_hit = true;
        }
    }

    return found_hit ? std::max(0.1f, nearest - 0.1f) : desired_distance;
}

} // namespace camera_collision
