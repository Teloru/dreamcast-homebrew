
#include "game.h"

void GameScene::on_load() {
    auto camera = create_child<smlt::Camera3D>();
    
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
