#include "game.h"

#include <string>

namespace {

struct MenuBinding {
    const char* name;
    smlt::JoystickButton button;
};

const MenuBinding MENU_BINDINGS[] = {
    {"menu.toggle", smlt::JOYSTICK_BUTTON_START},
    {"menu.cancel", smlt::JOYSTICK_BUTTON_B},
    {"menu.up", smlt::JOYSTICK_BUTTON_DPAD_UP},
    {"menu.down", smlt::JOYSTICK_BUTTON_DPAD_DOWN},
    {"menu.left", smlt::JOYSTICK_BUTTON_DPAD_LEFT},
    {"menu.right", smlt::JOYSTICK_BUTTON_DPAD_RIGHT},
    {"menu.confirm", smlt::JOYSTICK_BUTTON_A}
};

smlt::ui::Label* make_menu_label(smlt::Stage* parent, const char* text,
                                 float x, float y, int width, int height,
                                 int font_size, int precedence) {
    smlt::Params params;
    params.set("text", std::string(text));
    auto* label = parent->create_child<smlt::ui::Label>(params);
    label->set_font("Orbitron", smlt::ui::Px(font_size));
    label->resize(smlt::ui::Px(width), smlt::ui::Px(height));
    label->transform->set_position({x, y, 0.0f});
    label->set_background_color(smlt::Color::none());
    label->set_text_color(smlt::Color::white());
    label->set_precedence(precedence);
    return label;
}

} // namespace

void GameScene::bind_menu_controls() {
    // Each direction has its own positive axis, so presses are unambiguous.
    for(const auto& binding : MENU_BINDINGS) {
        auto button = input->new_axis(binding.name);
        button->set_positive_joystick_button(binding.button);
    }
}

void GameScene::unbind_menu_controls() {
    for(const auto& binding : MENU_BINDINGS) {
        input->destroy_axises(binding.name);
    }
}

void GameScene::setup_menu() {
    // Keep all menu geometry in the existing orthographic UI layer.
    menu_root_ = ui_root_->create_child<smlt::Stage>();
    auto* panel = make_menu_label(menu_root_, "", 96.0f, 80.0f,
                                  448, 320, 20, 30);
    panel->set_background_color(smlt::Color(0.06f, 0.10f, 0.19f, 1.0f));
    panel->set_border_width(smlt::ui::Px(2));
    panel->set_border_color(smlt::Color(0.9f, 0.9f, 0.9f, 1.0f));

    auto* title = make_menu_label(menu_root_, "PAUSE", 120.0f, 344.0f,
                                  400, 40, 24, 31);
    title->set_text_color(smlt::Color(1.0f, 0.85f, 0.35f, 1.0f));

    for(std::size_t i = 0; i < menu_rows_.size(); ++i) {
        menu_rows_[i] = make_menu_label(menu_root_, "", 120.0f,
            280.0f - 48.0f * i, 400, 40, 20, 31);
        menu_rows_[i]->set_text_alignment(smlt::ui::TEXT_ALIGNMENT_LEFT);
        menu_rows_[i]->set_padding(smlt::ui::Px(8));
    }
    make_menu_label(menu_root_,
        "Croix : choisir / regler\nA : valider   B / Start : reprendre",
        112.0f, 96.0f, 416, 64, 14, 31);

    update_menu_ui();
}

void GameScene::update_menu_ui() {
    menu_root_->set_visible(menu_.is_open());
    const std::array<std::string, 3> labels = {
        "Reprendre",
        "Volume musique : < " + std::to_string(menu_.music_volume()) + " % >",
        "Recommencer la quete"
    };
    for(std::size_t i = 0; i < menu_rows_.size(); ++i) {
        const bool selected = static_cast<std::size_t>(menu_.selected()) == i;
        menu_rows_[i]->set_text(::unicode((selected ? "> " : "  ") + labels[i]));
        menu_rows_[i]->set_text_color(selected
            ? smlt::Color(1.0f, 0.85f, 0.35f, 1.0f) : smlt::Color::white());
        menu_rows_[i]->set_background_color(selected
            ? smlt::Color(0.14f, 0.23f, 0.38f, 1.0f) : smlt::Color::none());
    }
}

bool GameScene::update_menu() {
    const bool was_open = menu_.is_open();
    const auto previous_item = menu_.selected();
    const auto previous_volume = menu_.music_volume();

    pause_menu::Input pressed;
    pressed.toggle = input->axis_was_pressed("menu.toggle");
    pressed.cancel = input->axis_was_pressed("menu.cancel");
    pressed.up = input->axis_was_pressed("menu.up");
    pressed.down = input->axis_was_pressed("menu.down");
    pressed.left = input->axis_was_pressed("menu.left");
    pressed.right = input->axis_was_pressed("menu.right");
    pressed.confirm = input->axis_was_pressed("menu.confirm");
    const auto action = menu_.update(pressed);

    if(!was_open && !menu_.is_open()) {
        return false;
    }
    if(action == pause_menu::Action::VOLUME_CHANGED) {
        apply_music_volume();
        play_sound_effect(SoundEffect::VOLUME_TICK);
    } else if(action == pause_menu::Action::RESUME) {
        play_sound_effect(SoundEffect::RESUME);
    } else if(action == pause_menu::Action::RESTART) {
        reset_player();
    }
    if(was_open != menu_.is_open() || previous_item != menu_.selected() ||
       previous_volume != menu_.music_volume()) {
        update_menu_ui();
    }
    if(was_open != menu_.is_open()) {
        update_ui();
    }
    return true;
}
