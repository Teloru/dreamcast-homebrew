#include "game.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float MOVE_SPEED = 3.5f;
constexpr float TURN_SPEED = 100.0f;
constexpr float CAMERA_DISTANCE = 5.5f;

const char* const FORWARD = "exploration.forward";
const char* const STRAFE = "exploration.strafe";
const char* const TURN = "exploration.turn";
const char* const TILT = "exploration.tilt";
const char* const RESET = "exploration.reset";
const char* const TURN_LEFT = "camera.turn_left";
const char* const TURN_RIGHT = "camera.turn_right";
const char* const INTERACT = "exploration.interact";

} // namespace

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
    auto actor = create_child<smlt::Actor>(make_box_mesh(size, material));
    actor->transform->set_position(position);

    if(blocks_player) {
        obstacles_.push_back({position.x, position.z, size.x * 0.5f,
                              size.z * 0.5f});
    }
    // Pad the boxes to keep the camera's near plane outside the geometry.
    const auto half_size = size * 0.5f + smlt::Vec3(0.2f, 0.2f, 0.2f);
    const auto min = position - half_size;
    const auto max = position + half_size;
    camera_obstacles_.push_back({{min.x, min.y, min.z}, {max.x, max.y, max.z}});
}

void GameScene::on_load() {
    obstacles_.clear();
    camera_obstacles_.clear();

    lighting->set_ambient_light(smlt::Color(0.3f, 0.3f, 0.3f, 1.0f));
    auto sun = create_child<smlt::DirectionalLight>();
    sun->set_direction(-0.6f, -1.0f, -0.4f);
    sun->set_color(smlt::Color::white());
    sun->set_intensity(0.8f);

    auto grass = make_material(smlt::Color(0.28f, 0.48f, 0.3f, 1.0f));
    auto light_grass = make_material(smlt::Color(0.38f, 0.58f, 0.37f, 1.0f));
    auto stone = make_material(smlt::Color(0.65f, 0.68f, 0.7f, 1.0f));
    auto clay = make_material(smlt::Color(0.85f, 0.4f, 0.2f, 1.0f));
    auto turquoise = make_material(smlt::Color(0.2f, 0.65f, 0.65f, 1.0f));

    // Reuse two meshes for the 8 x 8 floor tiles. The floor is at Y = 0.
    const smlt::Vec3 tile_size(4.0f, 0.1f, 4.0f);
    const smlt::MeshPtr tiles[] = {make_box_mesh(tile_size, grass),
                                  make_box_mesh(tile_size, light_grass)};
    for(int z = 0; z < 8; ++z) {
        for(int x = 0; x < 8; ++x) {
            auto tile = create_child<smlt::Actor>(tiles[(x + z) % 2]);
            tile->transform->set_position(smlt::Vec3(
                -walking::WORLD_HALF_SIZE + 2.0f + x * 4.0f, -0.05f,
                -walking::WORLD_HALF_SIZE + 2.0f + z * 4.0f));
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

    // Keep our original three-vertex triangle as a landmark on a pedestal.
    add_box({0.0f, 0.3f, -5.0f}, {2.4f, 0.6f, 1.2f}, stone);
    auto material = assets->load_material(smlt::Material::BuiltIns::DIFFUSE_ONLY);
    material->set_base_color(smlt::Color(1.0f, 0.8f, 0.1f, 1.0f));
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
    auto triangle = create_child<smlt::Actor>(mesh);
    triangle->transform->set_position({0.0f, 1.6f, -5.0f});

    // Group the body, head and nose so they move and turn together.
    player_ = create_child<smlt::Stage>();
    auto body = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.6f, 0.9f, 0.5f}, make_material({0.95f, 0.65f, 0.15f, 1.0f})));
    body->transform->set_translation({0.0f, 0.5f, 0.0f});
    auto head = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.45f, 0.45f, 0.45f}, make_material({0.9f, 0.9f, 0.85f, 1.0f})));
    head->transform->set_translation({0.0f, 1.2f, 0.0f});
    auto nose = player_->create_child<smlt::Actor>(make_box_mesh(
        {0.15f, 0.12f, 0.15f}, turquoise));
    nose->transform->set_translation({0.0f, 1.2f, -0.28f});

    camera_ = create_child<smlt::Camera3D>();
    camera_->set_perspective_projection(smlt::Degrees(60.0f),
                                        window->aspect_ratio(), 0.1f, 100.0f);
    auto layer = compositor->create_layer(this, camera_);
    layer->set_clear_flags(smlt::BUFFER_CLEAR_ALL);
    layer->viewport->set_color(smlt::Color(0.1f, 0.2f, 0.5f, 1.0f));

    bind_controls();
    reset_player();
}

void GameScene::bind_controls() {
    auto keys = [this](const char* name, smlt::KeyboardCode positive,
                       smlt::KeyboardCode negative = smlt::KEYBOARD_CODE_NONE) {
        auto axis = input->new_axis(name);
        axis->set_positive_keyboard_key(positive);
        axis->set_negative_keyboard_key(negative);
        axis->set_force(10.0f);
        axis->set_return_speed(20.0f);
    };
    auto buttons = [this](const char* name, smlt::JoystickButton positive,
                          smlt::JoystickButton negative = smlt::JOYSTICK_BUTTON_INVALID) {
        auto axis = input->new_axis(name);
        axis->set_positive_joystick_button(positive);
        axis->set_negative_joystick_button(negative);
        axis->set_force(10.0f);
        axis->set_return_speed(20.0f);
    };

    auto forward = input->new_axis(FORWARD);
    forward->set_joystick_axis(smlt::JOYSTICK_AXIS_Y);
    forward->set_dead_zone(0.18f);
    auto strafe = input->new_axis(STRAFE);
    strafe->set_joystick_axis(smlt::JOYSTICK_AXIS_X);
    strafe->set_dead_zone(0.18f);

    auto turn_left = input->new_axis(TURN_LEFT);
    turn_left->set_joystick_axis(smlt::JOYSTICK_AXIS_LTRIGGER);
    turn_left->set_dead_zone(0.18f);
    auto turn_right = input->new_axis(TURN_RIGHT);
    turn_right->set_joystick_axis(smlt::JOYSTICK_AXIS_RTRIGGER);
    turn_right->set_dead_zone(0.18f);

    keys(FORWARD, smlt::KEYBOARD_CODE_W, smlt::KEYBOARD_CODE_S);
    keys(FORWARD, smlt::KEYBOARD_CODE_Z, smlt::KEYBOARD_CODE_S);
    keys(STRAFE, smlt::KEYBOARD_CODE_D, smlt::KEYBOARD_CODE_A);
    keys(STRAFE, smlt::KEYBOARD_CODE_D, smlt::KEYBOARD_CODE_Q);
    keys(TURN, smlt::KEYBOARD_CODE_RIGHT, smlt::KEYBOARD_CODE_LEFT);
    keys(TILT, smlt::KEYBOARD_CODE_UP, smlt::KEYBOARD_CODE_DOWN);
    keys(RESET, smlt::KEYBOARD_CODE_R);
    keys(INTERACT, smlt::KEYBOARD_CODE_E);

    buttons(TURN, smlt::JOYSTICK_BUTTON_DPAD_RIGHT, smlt::JOYSTICK_BUTTON_DPAD_LEFT);
    buttons(TILT, smlt::JOYSTICK_BUTTON_DPAD_UP, smlt::JOYSTICK_BUTTON_DPAD_DOWN);
    buttons(RESET, smlt::JOYSTICK_BUTTON_START);
    buttons(INTERACT, smlt::JOYSTICK_BUTTON_A);
}

void GameScene::reset_player() {
    player_->transform->set_position({0.0f, 0.0f, 11.0f});
    player_->transform->set_orientation(smlt::Quaternion());
    camera_yaw_ = 0.0f;
    camera_elevation_ = 30.0f;
    update_camera();
}

void GameScene::update_camera() {
    const auto target = player_->transform->position() + smlt::Vec3(0.0f, 1.0f, 0.0f);
    const auto orbit = smlt::Quaternion(smlt::Vec3::up(), smlt::Degrees(camera_yaw_));
    const float elevation = smlt::Degrees(camera_elevation_).to_radians().to_float();
    const auto direction = -orbit.forward() * std::cos(elevation) +
                            smlt::Vec3::up() * std::sin(elevation);

    // Pull the camera toward the player when a wall blocks the view.
    const float distance = camera_collision::limit_distance(
        {target.x, target.y, target.z}, {direction.x, direction.y, direction.z},
        CAMERA_DISTANCE, camera_obstacles_);
    camera_->transform->set_position(target + direction * distance);
    camera_->transform->look_at(target);
}

void GameScene::on_update(float dt) {
    if(input->axis_was_pressed(RESET)) {
        reset_player();
        return;
    }

    // Avoid a large jump after a pause; speeds are expressed per second.
    dt = std::clamp(dt, 0.0f, 0.1f);
    const float turn =
        input->axis_value(TURN) +
        input->axis_value(TURN_RIGHT) -
        input->axis_value(TURN_LEFT);
    camera_yaw_ = std::remainder(camera_yaw_ + turn * TURN_SPEED * dt, 360.0f);
    camera_elevation_ = std::clamp(
        camera_elevation_ + input->axis_value(TILT) * 60.0f * dt, 15.0f, 65.0f);

    // Movement uses only the camera yaw, so tilting never lifts the player.
    const auto yaw = smlt::Quaternion(smlt::Vec3::up(), smlt::Degrees(camera_yaw_));
    auto movement = yaw.forward() * input->axis_value(FORWARD) +
                    yaw.right() * input->axis_value(STRAFE);
    if(movement.length_squared() > 1.0f) {
        movement.normalize();
    }

    const auto old_position = player_->transform->position();
    const auto position = walking::move({old_position.x, old_position.z},
        movement.x * MOVE_SPEED * dt, movement.z * MOVE_SPEED * dt, obstacles_);
    player_->transform->set_position({position.x, 0.0f, position.z});

    if(movement.length_squared() > 0.0001f) {
        const auto facing =
            smlt::Radians(std::atan2(-movement.x, -movement.z)).to_degrees();
        player_->transform->set_orientation(
            smlt::Quaternion(smlt::Vec3::up(), facing));
    }
    update_camera();
}

void GameScene::on_unload() {
    // InputManager outlives this scene: remove only the bindings we created.
    for(const auto* name : {FORWARD, STRAFE, TURN, TILT, RESET,
                            TURN_LEFT, TURN_RIGHT, INTERACT}) {
        input->destroy_axises(name);
    }
    player_ = nullptr;
    camera_ = nullptr;
    obstacles_.clear();
    camera_obstacles_.clear();
}
