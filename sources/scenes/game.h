#pragma once

#include <simulant/simulant.h>

#include "camera_collision.h"
#include "walking.h"

class GameScene : public smlt::Scene {
public:
    GameScene(smlt::Window* window):
        smlt::Scene(window) {}

    void on_load() override;
    void on_update(float dt) override;
    void on_unload() override;

private:
    smlt::MaterialPtr make_material(const smlt::Color& color);
    smlt::MeshPtr make_box_mesh(const smlt::Vec3& size,
                               const smlt::MaterialPtr& material);
    void add_box(const smlt::Vec3& position, const smlt::Vec3& size,
                 const smlt::MaterialPtr& material, bool blocks_player = true);
    void bind_controls();
    void reset_player();
    void update_camera();

    smlt::Stage* player_ = nullptr;
    smlt::Camera3D* camera_ = nullptr;
    float camera_yaw_ = 0.0f;
    float camera_elevation_ = 30.0f;
    std::vector<walking::Obstacle> obstacles_;
    std::vector<camera_collision::Box> camera_obstacles_;
};
