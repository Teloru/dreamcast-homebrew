#include <simulant/simulant.h>

#include "scenes/game.h"

using namespace smlt;

class DreamcastNext:
    public Application {

public:
    DreamcastNext(const AppConfig& config, void* platform_state):
        Application(config, platform_state) {}

    bool init() {
        // Register screens here
        scenes->register_scene<smlt::scenes::Splash>("main", "app");
        scenes->register_scene<GameScene>("app");

        scenes->signal_scene_activated().connect(
            [](const std::string& route, Scene* scene) {
                if(route != "main") {
                    return;
                }

                for(auto* image : scene->nodes_by_type(ui::Image::Meta::node_type)) {
                    if(!image->has_parent()) {
                        image->set_parent(scene);
                    }
                }
            });

        return true;
    }
};

extern "C" {

#ifdef __ANDROID__
// Android's NativeActivity starts the app here, on its own thread, handing
// over the app state that Simulant needs to get at the window and assets
void android_main(struct android_app* platform_state) {
#else
int main(int argc, char* argv[]) {
    void* platform_state = nullptr;
#endif
    AppConfig config;
    config.title = "dreamcast-next";
    config.search_paths.push_back("assets");

    // For Flatpak
    config.search_paths.push_back(kfs::exe_dirname() + "/assets");

    // Dreamcast only
    config.search_paths.push_back("/cd/assets");

#ifndef NDEBUG
    config.log_level = smlt::LOG_LEVEL_DEBUG;
#endif

    DreamcastNext app(config, platform_state);
#ifdef __ANDROID__
    app.run();
#else
    return app.run();
#endif
}

}
