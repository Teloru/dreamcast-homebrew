#pragma once

#include <simulant/simulant.h>

#include <array>

#include "camera_collision.h"
#include "interaction.h"
#include "pause_menu.h"
#include "walking.h"

enum class QuestState {
    NOT_STARTED,
    SEARCHING_ITEM,
    ITEM_FOUND,
    ITEM_REPAIRED,
    COMPLETED
};

enum class DialogueAction {
    NONE,
    START_QUEST,
    FIND_ITEM,
    REPAIR_ITEM,
    COMPLETE_QUEST
};

class GameScene : public smlt::Scene {
public:
    GameScene(smlt::Window* window):
        smlt::Scene(window) {}

    void on_load() override;
    void on_update(float dt) override;
    void on_unload() override;

private:
    enum class SoundEffect { QUESTION, STAR, VOLUME_TICK, RESUME, COUNT };

    void create_world();
    smlt::MaterialPtr make_material(const smlt::Color& color);
    smlt::MeshPtr make_box_mesh(const smlt::Vec3& size,
                               const smlt::MaterialPtr& material);
    void add_box(const smlt::Vec3& position, const smlt::Vec3& size,
                 const smlt::MaterialPtr& material, bool blocks_player = true);
    void add_triangle_landmark(const smlt::Vec3& position);
    void add_camera_obstacle(const smlt::Vec3& position, const smlt::Vec3& size);
    void bind_controls();
    void start_music();
    void stop_music();
    void apply_music_volume();
    void load_sound_effects();
    void play_sound_effect(SoundEffect effect);
    void unload_sound_effects();
    void bind_menu_controls();
    void unbind_menu_controls();
    void setup_menu();
    void update_menu_ui();
    bool update_menu();
    void reset_player();
    void update_camera();
    void setup_ui();
    void update_ui();
    bool update_nearby_interaction();
    void interact_with_nearby();
    void update_dialogue_camera();
    void begin_dialogue(const char* name, const char* const* lines,
                        std::size_t line_count,
                        DialogueAction action = DialogueAction::NONE);
    void advance_dialogue();
    float ground_height(float old_x, float old_z,
                        float new_x, float new_z,
                        float current_height) const;

    smlt::Stage* world_root_ = nullptr;
    smlt::Stage* ui_root_ = nullptr;
    smlt::Stage* player_ = nullptr;
    smlt::Stage* npc_a_ = nullptr;
    smlt::Stage* npc_b_ = nullptr;
    smlt::Camera3D* camera_ = nullptr;
    smlt::Camera2D* ui_camera_ = nullptr;
    smlt::ui::Label* interaction_label_ = nullptr;
    smlt::ui::Label* dialogue_name_ = nullptr;
    smlt::ui::Label* dialogue_text_ = nullptr;
    smlt::Stage* menu_root_ = nullptr;
    std::array<smlt::ui::Label*, 3> menu_rows_{};
    pause_menu::State menu_;
    smlt::Actor* object_ = nullptr;
    smlt::Actor* victory_marker_ = nullptr;
    smlt::SoundPtr music_;
    smlt::AudioSource* music_source_ = nullptr;
    smlt::PlayingSoundPtr music_playing_;
    std::array<smlt::SoundPtr, static_cast<std::size_t>(SoundEffect::COUNT)> sound_effects_{};
    smlt::AudioSource* sound_effects_source_ = nullptr;
    float camera_yaw_ = 0.0f;
    float camera_elevation_ = 30.0f;
    float player_height_ = 0.0f;
    QuestState quest_state_ = QuestState::NOT_STARTED;
    bool dialogue_open_ = false;
    std::size_t dialogue_index_ = 0;
    std::size_t dialogue_count_ = 0;
    const char* dialogue_name_text_ = "";
    const char* const* dialogue_lines_ = nullptr;
    interaction::Target nearby_interaction_ = interaction::Target::NONE;
    DialogueAction dialogue_action_ = DialogueAction::NONE;
    std::string ui_interaction_text_;
    std::string ui_dialogue_name_;
    std::string ui_dialogue_text_;
    bool dialogue_ui_visible_ = false;
    bool interaction_ui_visible_ = false;
    std::vector<walking::Obstacle> obstacles_;
    std::vector<camera_collision::Box> camera_obstacles_;
    struct PlatformSurface {
        float x;
        float z;
        float half_width;
        float half_depth;
        float top;
    };
    std::vector<PlatformSurface> platforms_;
};
