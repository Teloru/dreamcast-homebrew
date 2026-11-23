#include "game.h"

#include <algorithm>
#include <cmath>

smlt::MaterialPtr GameScene::make_material(const smlt::Color& color) {
    auto material = assets->load_material(smlt::Material::BuiltIns::DEFAULT);
    material->set_base_color(color);
    material->set_textures_enabled(0);
    material->set_blend_func(smlt::BLEND_NONE);
    material->set_roughness(0.9f);
    return material;
}

smlt::MeshPtr GameScene::make_box_mesh(const smlt::Vec3& size,
                                     const smlt::MaterialPtr& material) {
    // DEFAULT includes the normals needed to shade the faces.
    auto mesh = assets->create_mesh(smlt::VertexSpecification::DEFAULT);
    mesh->create_submesh_as_box("box", material, size.x, size.y, size.z);
    return mesh;
}

void GameScene::add_box(const smlt::Vec3& position, const smlt::Vec3& size,
                        const smlt::MaterialPtr& material, bool blocks_player) {
    auto actor = world_root_->create_child<smlt::Actor>(make_box_mesh(size, material));
    actor->transform->set_position(position);

    if(blocks_player) {
        obstacles_.push_back({position.x, position.z, size.x * 0.5f,
                              size.z * 0.5f});
    }
    add_camera_obstacle(position, size);
}

void GameScene::add_triangle_landmark(const smlt::Vec3& position) {
    // Preserve the first hand-written mesh as a landmark in the world.
    auto material = assets->load_material(smlt::Material::BuiltIns::DIFFUSE_ONLY);
    material->set_base_color(smlt::Color(1.0f, 0.8f, 0.1f, 1.0f));
    material->set_cull_mode(smlt::CULL_MODE_NONE);

    auto mesh = assets->create_mesh(smlt::VertexSpecification::POSITION_ONLY);
    auto vertices = mesh->vertex_data.get();
    vertices->position(-1.0f, -1.0f, 0.0f);
    vertices->move_next();
    vertices->position(1.0f, -1.0f, 0.0f);
    vertices->move_next();
    vertices->position(0.0f, 1.0f, 0.0f);
    vertices->move_next();
    vertices->done();

    auto submesh = mesh->create_submesh("triangle", material,
                                      smlt::MESH_ARRANGEMENT_TRIANGLES);
    submesh->add_vertex_range(0, 3);
    auto triangle = world_root_->create_child<smlt::Actor>(mesh);
    triangle->transform->set_position(position);

    const auto bounds = triangle->transformed_aabb();
    add_camera_obstacle(bounds.center(), bounds.dimensions());
}

void GameScene::add_camera_obstacle(const smlt::Vec3& position,
                                    const smlt::Vec3& size) {
    // Pad the boxes to keep the camera's near plane outside the geometry.
    const auto half_size = size * 0.5f + smlt::Vec3(0.2f, 0.2f, 0.2f);
    const auto min = position - half_size;
    const auto max = position + half_size;
    camera_obstacles_.push_back({{min.x, min.y, min.z}, {max.x, max.y, max.z}});
}

void GameScene::create_world() {
    // Each camera renders its own subtree; UI panels must stay out of 3D.
    world_root_ = create_child<smlt::Stage>();

    lighting->set_ambient_light(smlt::Color(0.3f, 0.3f, 0.3f, 1.0f));
    auto sun = world_root_->create_child<smlt::DirectionalLight>();
    sun->set_direction(-0.6f, -1.0f, -0.4f);
    sun->set_color(smlt::Color::white());
    sun->set_intensity(0.8f);

    auto grass = make_material(smlt::Color(0.28f, 0.48f, 0.3f, 1.0f));
    auto stone = make_material(smlt::Color(0.65f, 0.68f, 0.7f, 1.0f));
    auto clay = make_material(smlt::Color(0.85f, 0.4f, 0.2f, 1.0f));
    auto turquoise = make_material(smlt::Color(0.2f, 0.65f, 0.65f, 1.0f));

    // Split the floor into four large slabs. A single 32 x 32 primitive can
    // expose PVR cracks when the camera looks across it from a high platform.
    // This keeps 60 render nodes instead of 64 floor tiles but avoids
    // submitting an oversized primitive.
    for(int z = -1; z <= 1; z += 2) {
        for(int x = -1; x <= 1; x += 2) {
            auto floor = world_root_->create_child<smlt::Actor>(
                make_box_mesh({16.0f, 0.1f, 16.0f}, grass));
            floor->transform->set_position(
                {static_cast<float>(x) * 8.0f, -0.05f,
                 static_cast<float>(z) * 8.0f});
        }
    }

    // Low boundary walls and a few landmarks leave space to walk around.
    add_box({0.0f, 0.6f, -15.8f}, {32.0f, 1.2f, 0.4f}, stone);
    add_box({0.0f, 0.6f, 15.8f}, {32.0f, 1.2f, 0.4f}, stone);
    add_box({-15.8f, 0.6f, 0.0f}, {0.4f, 1.2f, 32.0f}, stone);
    add_box({15.8f, 0.6f, 0.0f}, {0.4f, 1.2f, 32.0f}, stone);

    add_box({-7.0f, 1.5f, -3.0f}, {3.0f, 3.0f, 3.0f}, clay);
    add_box({-10.0f, 0.75f, 5.0f}, {2.0f, 1.5f, 3.0f}, clay);
    add_box({-5.0f, 0.5f, 5.0f}, {2.0f, 1.0f, 2.0f}, stone);
    add_box({8.0f, 2.0f, 4.0f}, {2.0f, 4.0f, 2.0f}, turquoise);
    add_box({10.0f, 1.0f, -1.0f}, {3.0f, 2.0f, 2.0f}, turquoise);

    // An arch: its beam blocks the camera, but the player can walk under it.
    add_box({4.0f, 1.75f, -9.0f}, {1.0f, 3.5f, 1.0f}, stone);
    add_box({8.0f, 1.75f, -9.0f}, {1.0f, 3.5f, 1.0f}, stone);
    add_box({6.0f, 3.75f, -9.0f}, {5.0f, 0.5f, 1.0f}, stone, false);

    // Keep our original yellow triangle on its pedestal.
    add_box({0.0f, 0.3f, -5.0f}, {2.4f, 0.6f, 1.2f}, stone);
    add_triangle_landmark({0.0f, 1.6f, -5.0f});

    // Simple stepped platform for the quest object. It intentionally uses
    // the existing X/Z collision and updates only the player's ground height.
    add_box({12.0f, 0.375f, -13.0f}, {4.0f, 0.75f, 2.0f},
            stone, false);
    platforms_.push_back({12.0f, -13.0f, 2.0f, 1.0f, 0.75f});
    add_box({12.0f, 1.125f, -11.0f}, {4.0f, 0.75f, 2.0f},
            stone, false);
    platforms_.push_back({12.0f, -11.0f, 2.0f, 1.0f, 1.5f});
    add_box({12.0f, 1.875f, -9.0f}, {4.0f, 0.75f, 2.0f},
            stone, false);
    platforms_.push_back({12.0f, -9.0f, 2.0f, 1.0f, 2.25f});
    add_box({12.0f, 2.625f, -7.0f}, {4.0f, 0.75f, 2.0f},
            clay, false);
    platforms_.push_back({12.0f, -7.0f, 2.0f, 1.0f, 3.0f});

    // Two primitive NPCs and the recoverable quest object.
    npc_a_ = world_root_->create_child<smlt::Stage>();
    npc_a_->transform->set_position({-6.0f, 0.0f, 10.0f});
    auto npc_a_body = npc_a_->create_child<smlt::Actor>(make_box_mesh(
        {0.8f, 1.8f, 0.8f}, make_material({0.75f, 0.25f, 0.75f, 1.0f})));
    npc_a_body->transform->set_translation({0.0f, 0.9f, 0.0f});

    npc_b_ = world_root_->create_child<smlt::Stage>();
    npc_b_->transform->set_position({-12.0f, 0.0f, -12.0f});
    auto npc_b_body = npc_b_->create_child<smlt::Actor>(make_box_mesh(
        {0.8f, 1.8f, 0.8f}, make_material({0.25f, 0.75f, 0.85f, 1.0f})));
    npc_b_body->transform->set_translation({0.0f, 0.9f, 0.0f});

    object_ = world_root_->create_child<smlt::Actor>(make_box_mesh(
        {0.5f, 0.5f, 0.5f},
        make_material({1.0f, 0.45f, 0.05f, 1.0f})));
    object_->transform->set_position({12.0f, 3.25f, -7.0f});
    object_->set_visible(false);

    victory_marker_ = world_root_->create_child<smlt::Actor>(make_box_mesh(
        {1.2f, 0.2f, 1.2f},
        make_material({0.25f, 1.0f, 0.25f, 1.0f})));
    victory_marker_->transform->set_position({-6.0f, 0.1f, 8.0f});
    victory_marker_->set_visible(false);

    // Group the body, head and nose so they move and turn together.
    player_ = world_root_->create_child<smlt::Stage>();
    auto body = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.6f, 0.9f, 0.5f}, make_material({0.95f, 0.65f, 0.15f, 1.0f})));
    body->transform->set_translation({0.0f, 0.5f, 0.0f});
    auto head = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.45f, 0.45f, 0.45f}, make_material({0.9f, 0.9f, 0.85f, 1.0f})));
    head->transform->set_translation({0.0f, 1.2f, 0.0f});
    auto nose = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.15f, 0.12f, 0.15f}, turquoise));
    nose->transform->set_translation({0.0f, 1.2f, -0.28f});
}

float GameScene::ground_height(float old_x, float old_z,
                               float new_x, float new_z,
                               float current_height) const {
    float height = current_height;

    if(current_height > 0.0f) {
        bool standing_on_surface = false;
        for(const auto& platform : platforms_) {
            if(std::abs(new_x - platform.x) <= platform.half_width &&
               std::abs(new_z - platform.z) <= platform.half_depth &&
               std::abs(current_height - platform.top) < 0.01f) {
                standing_on_surface = true;
                break;
            }
        }
        if(!standing_on_surface) {
            height = 0.0f;
            for(const auto& platform : platforms_) {
                if(std::abs(new_x - platform.x) <= platform.half_width &&
                   std::abs(new_z - platform.z) <= platform.half_depth &&
                   platform.top <= current_height + 0.75f) {
                    height = std::max(height, platform.top);
                }
            }
        }
    }

    float highest_step = height;
    for(const auto& platform : platforms_) {
        if(platform.top <= current_height + 0.75f &&
           platform.top > highest_step + 0.01f &&
           std::abs(new_x - platform.x) <= platform.half_width &&
           std::abs(new_z - platform.z) <= platform.half_depth) {
            const bool entered_from_x =
                (std::abs(old_x - (platform.x - platform.half_width)) < 0.9f ||
                 std::abs(old_x - (platform.x + platform.half_width)) < 0.9f) &&
                std::abs(new_z - platform.z) <= platform.half_depth;
            const bool entered_from_z =
                (std::abs(old_z - (platform.z - platform.half_depth)) < 0.9f ||
                 std::abs(old_z - (platform.z + platform.half_depth)) < 0.9f) &&
                std::abs(new_x - platform.x) <= platform.half_width;
            if(entered_from_x || entered_from_z) {
                highest_step = platform.top;
            }
        }
    }
    return highest_step;
}
