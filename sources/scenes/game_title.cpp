#include "game.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float SHOT_DURATION = 8.0f;

struct CameraShot {
    smlt::Vec3 from;
    smlt::Vec3 to;
    smlt::Vec3 target;
};

const std::array<CameraShot, 3> SHOTS = {{
    {{14.0f, 10.0f, 15.0f}, {10.0f, 8.0f, 13.0f}, {0.0f, 0.8f, 0.0f}},
    {{3.5f, 3.2f, 1.0f}, {-3.5f, 2.5f, 0.0f}, {0.0f, 1.6f, -5.0f}},
    {{14.0f, 6.0f, 2.0f}, {10.0f, 5.5f, 0.5f}, {10.0f, 2.0f, -8.0f}}
}};

} // namespace

void GameScene::setup_title() {
    title_root_ = ui_root_->create_child<smlt::Stage>();
    title_heading_ = make_menu_label(title_root_, "Bun's Day Out",
        80.0f, 384.0f, 480, 44, 28, 41);
    title_heading_->set_text_color(smlt::Color(1.0f, 0.85f, 0.35f, 1.0f));
    title_heading_->set_background_color(smlt::Color(0.06f, 0.10f, 0.19f, 0.9f));
    title_prompt_ = make_menu_label(title_root_, "Appuie sur Start",
        112.0f, 64.0f, 416, 44, 22, 41);
    title_prompt_->set_background_color(smlt::Color(0.06f, 0.10f, 0.19f, 0.9f));

    title_panel_ = make_menu_label(title_root_, "", 96.0f, 96.0f, 448, 232, 20, 40);
    title_panel_->set_background_color(smlt::Color(0.06f, 0.10f, 0.19f, 1.0f));
    title_panel_->set_border_width(smlt::ui::Px(2));
    title_panel_->set_border_color(smlt::Color(0.9f, 0.9f, 0.9f, 1.0f));
    for(std::size_t i = 0; i < title_rows_.size(); ++i) {
        title_rows_[i] = make_menu_label(title_root_, "", 120.0f,
            242.0f - 56.0f * i, 400, 44, 20, 41);
        title_rows_[i]->set_text_alignment(smlt::ui::TEXT_ALIGNMENT_LEFT);
        title_rows_[i]->set_padding(smlt::ui::Px(8));
    }
    title_help_ = make_menu_label(title_root_, "", 112.0f, 112.0f, 416, 48, 14, 41);
    update_title_ui();
}

void GameScene::show_title() {
    menu_.close();
    title_.show_prompt();
    title_camera_time_ = 0.0f;
    reset_player();
    player_->set_visible(false);
    update_title_camera(0.0f);
    update_menu_ui();
    update_title_ui();
}

void GameScene::update_title_camera(float dt) {
    title_camera_time_ = std::fmod(title_camera_time_ + std::clamp(dt, 0.0f, 0.1f),
                                  SHOT_DURATION * SHOTS.size());
    const auto index = static_cast<std::size_t>(title_camera_time_ / SHOT_DURATION);
    const auto& shot = SHOTS[index];
    const float t = std::fmod(title_camera_time_, SHOT_DURATION) / SHOT_DURATION;
    const float smooth = t * t * (3.0f - 2.0f * t);
    camera_->transform->set_position(shot.from + (shot.to - shot.from) * smooth);
    camera_->transform->look_at(shot.target);
}

void GameScene::update_title_ui() {
    const bool active = title_.is_active();
    const bool prompt = title_.screen() == title_menu::Screen::PROMPT;
    const bool options = title_.screen() == title_menu::Screen::OPTIONS;
    title_root_->set_visible(active);
    title_heading_->set_text(::unicode(options ? "OPTIONS" : "DREAMCAST NEXT"));
    title_prompt_->set_visible(prompt);
    title_panel_->set_visible(!prompt);
    title_help_->set_visible(!prompt);
    title_help_->set_text(::unicode(options
        ? "Croix : choisir / regler\nA : valider   B : retour"
        : "Croix : choisir   A : valider\nB : retour"));

    const std::array<std::string, 2> labels = options
        ? std::array<std::string, 2>{
            "Volume musique : < " + std::to_string(menu_.music_volume()) + " % >", "Retour"}
        : std::array<std::string, 2>{"Nouvelle partie", "Options"};
    for(std::size_t i = 0; i < title_rows_.size(); ++i) {
        title_rows_[i]->set_visible(!prompt);
        set_menu_row(title_rows_[i], labels[i], title_.selected() == i);
    }
}

bool GameScene::update_title(float dt) {
    if(!title_.is_active()) {
        return false;
    }
    update_title_camera(dt);
    const auto previous_screen = title_.screen();
    const auto previous_item = title_.selected();
    const auto previous_volume = menu_.music_volume();
    const auto action = title_.update(read_menu_input());

    if(action == title_menu::Action::START_GAME) {
        menu_.close();
        reset_player();
        player_->set_visible(true);
        play_sound_effect(SoundEffect::SCENE_CHANGE);
    } else if(action == title_menu::Action::VOLUME_DOWN ||
              action == title_menu::Action::VOLUME_UP) {
        const int direction = action == title_menu::Action::VOLUME_UP ? 1 : -1;
        if(menu_.change_music_volume(direction)) {
            apply_music_volume();
            play_sound_effect(SoundEffect::VOLUME_TICK);
        }
    } else if(action == title_menu::Action::BACK) {
        play_sound_effect(SoundEffect::RESUME);
    }

    if(previous_screen != title_.screen() || previous_item != title_.selected() ||
       previous_volume != menu_.music_volume()) {
        update_title_ui();
    }
    // Consume Start/A even on the frame that switches to gameplay.
    return true;
}
