
#include "game.h"

void GameScene::on_load() {
    // A solid, unlit material: no texture, light or vertex normals needed.
    auto material = assets->load_material(smlt::Material::BuiltIns::DIFFUSE_ONLY);
    material->set_base_color(smlt::Color(1.0f, 0.8f, 0.1f, 1.0f));

    auto mesh = assets->create_mesh(smlt::VertexSpecification::POSITION_ONLY);
    auto vertices = mesh->vertex_data.get();

    // Exactly three XYZ positions, counterclockwise when viewed from +Z.
    vertices->position(-1.0f, -1.0f, 0.0f);
    vertices->move_next();
    vertices->position(1.0f, -1.0f, 0.0f);
    vertices->move_next();
    vertices->position(0.0f, 1.0f, 0.0f);
    vertices->move_next();
    vertices->done();

    // Draw vertices 0, 1 and 2 as one triangle using this material.
    auto submesh = mesh->create_submesh("triangle", material,
                                      smlt::MESH_ARRANGEMENT_TRIANGLES);
    submesh->add_vertex_range(0, 3);
    create_child<smlt::Actor>(mesh);

    auto camera = create_child<smlt::Camera3D>();
    camera->set_perspective_projection(smlt::Degrees(60.0f),
                                       window->aspect_ratio(), 0.1f, 100.0f);
    camera->transform->set_position(smlt::Vec3(0.0f, 0.0f, 3.0f));
    camera->transform->look_at(smlt::Vec3::zero());

    auto layer = compositor->create_layer(this, camera);
    layer->set_clear_flags(smlt::BUFFER_CLEAR_ALL);
    // test: setting a blue viewport
    layer->viewport->set_color(smlt::Color(0.1f, 0.2f, 0.5f, 1.0f));
}

void GameScene::on_update(float dt) {
    // Update your scene here, called every frame
}

void GameScene::on_activate() {
    // Called when the scene is made active (after load)
}

void GameScene::on_deactivate() {
    // Called before unloading
}

void GameScene::on_unload() {
    // Cleanup your scene here
}
