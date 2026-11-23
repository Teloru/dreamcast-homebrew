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

const char* const NPC_A_DIALOGUE_START[] = {
    "J'ai perdu une pièce mécanique sur la plateforme.",
    "Elle doit encore être complète. Tu peux la chercher ?"
};
const char* const NPC_A_DIALOGUE_SEARCH[] = {
    "La plateforme est accessible par les marches à droite.",
    "Reviens me voir quand tu auras trouvé la pièce."
};
const char* const NPC_A_DIALOGUE_FOUND[] = {
    "Oh... elle est abîmée.",
    "Va voir Momo. Il sait certainement la remettre en état."
};
const char* const NPC_A_DIALOGUE_REPAIR[] = {
    "Tu as retrouvé Momo !",
    "Merci pour ton aide. Cette fois, elle est comme neuve."
};
const char* const NPC_B_DIALOGUE_REPAIR[] = {
    "Une pièce endommagée ? Ce n'est pas très compliqué.",
    "Tiens, elle est réparée."
};
const char* const NPC_B_DIALOGUE_WAIT[] = {
    "Je ne peux pas t'aider pour l'instant.",
    "Va voir Mia, elle a besoin de ton aide."
};
const char* const NPC_A_DIALOGUE_COMPLETE[] = {
    "Merci, tout est terminé.",
    "Tu peux repartir explorer."
};
const char* const OBJECT_DIALOGUE[] = {
    "Tu récupères une pièce cassée.",
    "Il manque un petit morceau au mécanisme."
};

interaction::Position position_xz(smlt::StageNode* node) {
    const auto position = node->transform->position();
    return {position.x, position.z};
}

} // namespace

void GameScene::on_load() {
    obstacles_.clear();
    camera_obstacles_.clear();
    platforms_.clear();
    quest_state_ = QuestState::NOT_STARTED;
    dialogue_open_ = false;
    nearby_interaction_ = interaction::Target::NONE;
    player_height_ = 0.0f;

    create_world();

    camera_ = create_child<smlt::Camera3D>();
    camera_->set_perspective_projection(smlt::Degrees(60.0f),
                                        window->aspect_ratio(), 0.1f, 100.0f);
    auto layer = compositor->create_layer(world_root_, camera_);
    layer->set_clear_flags(smlt::BUFFER_CLEAR_ALL);
    layer->viewport->set_color(smlt::Color(0.1f, 0.2f, 0.5f, 1.0f));

    setup_ui();

    bind_controls();
    reset_player();
    start_music();
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
    player_height_ = 0.0f;
    quest_state_ = QuestState::NOT_STARTED;
    dialogue_open_ = false;
    dialogue_action_ = DialogueAction::NONE;
    nearby_interaction_ = interaction::Target::NONE;
    ui_interaction_text_.clear();
    ui_dialogue_name_.clear();
    ui_dialogue_text_.clear();
    if(object_) {
        object_->set_visible(false);
    }
    if(victory_marker_) {
        victory_marker_->set_visible(false);
    }
    camera_yaw_ = 0.0f;
    camera_elevation_ = 30.0f;
    update_camera();
    update_ui();
}

void GameScene::begin_dialogue(const char* name, const char* const* lines,
                               std::size_t line_count,
                               DialogueAction action) {
    dialogue_open_ = true;
    dialogue_index_ = 0;
    dialogue_count_ = line_count;
    dialogue_name_text_ = name;
    dialogue_lines_ = lines;
    dialogue_action_ = action;
    update_dialogue_camera();
    update_ui();
}

void GameScene::advance_dialogue() {
    if(!dialogue_open_) {
        return;
    }

    if(dialogue_index_ + 1 < dialogue_count_) {
        ++dialogue_index_;
        update_ui();
        return;
    }

    dialogue_open_ = false;
    if(dialogue_action_ == DialogueAction::START_QUEST) {
        quest_state_ = QuestState::SEARCHING_ITEM;
        if(object_) {
            object_->set_visible(true);
        }
    } else if(dialogue_action_ == DialogueAction::FIND_ITEM) {
        quest_state_ = QuestState::ITEM_FOUND;
        if(object_) {
            object_->set_visible(false);
        }
    } else if(dialogue_action_ == DialogueAction::REPAIR_ITEM) {
        quest_state_ = QuestState::ITEM_REPAIRED;
    } else if(dialogue_action_ == DialogueAction::COMPLETE_QUEST) {
        quest_state_ = QuestState::COMPLETED;
        if(victory_marker_) {
            victory_marker_->set_visible(true);
        }
    }
    dialogue_action_ = DialogueAction::NONE;
    update_ui();
}

bool GameScene::update_nearby_interaction() {
    const auto previous = nearby_interaction_;
    nearby_interaction_ = interaction::Target::NONE;

    if(!dialogue_open_) {
        const bool object_available = quest_state_ == QuestState::SEARCHING_ITEM &&
                                       object_ && object_->is_visible();
        const auto object_position = object_available
            ? position_xz(object_) : interaction::Position{};
        nearby_interaction_ = interaction::find_nearby(
            position_xz(player_), position_xz(npc_a_), position_xz(npc_b_),
            object_position, object_available);
    }
    return previous != nearby_interaction_;
}

void GameScene::interact_with_nearby() {
    if(nearby_interaction_ == interaction::Target::NPC_A) {
        if(quest_state_ == QuestState::NOT_STARTED) {
            begin_dialogue("Mia",
                           NPC_A_DIALOGUE_START,
                           sizeof(NPC_A_DIALOGUE_START) /
                               sizeof(NPC_A_DIALOGUE_START[0]),
                           DialogueAction::START_QUEST);
        } else if(quest_state_ == QuestState::SEARCHING_ITEM) {
            begin_dialogue("Mia", NPC_A_DIALOGUE_SEARCH,
                           sizeof(NPC_A_DIALOGUE_SEARCH) /
                               sizeof(NPC_A_DIALOGUE_SEARCH[0]));
        } else if(quest_state_ == QuestState::ITEM_FOUND) {
            begin_dialogue("Mia", NPC_A_DIALOGUE_FOUND,
                           sizeof(NPC_A_DIALOGUE_FOUND) /
                               sizeof(NPC_A_DIALOGUE_FOUND[0]));
        } else if(quest_state_ == QuestState::ITEM_REPAIRED) {
            begin_dialogue("Mia", NPC_A_DIALOGUE_REPAIR,
                           sizeof(NPC_A_DIALOGUE_REPAIR) /
                               sizeof(NPC_A_DIALOGUE_REPAIR[0]),
                           DialogueAction::COMPLETE_QUEST);
        } else {
            begin_dialogue("Mia", NPC_A_DIALOGUE_COMPLETE,
                           sizeof(NPC_A_DIALOGUE_COMPLETE) /
                               sizeof(NPC_A_DIALOGUE_COMPLETE[0]));
        }
    } else if(nearby_interaction_ == interaction::Target::NPC_B) {
        if(quest_state_ == QuestState::ITEM_FOUND) {
            begin_dialogue("Momo", NPC_B_DIALOGUE_REPAIR,
                           sizeof(NPC_B_DIALOGUE_REPAIR) /
                               sizeof(NPC_B_DIALOGUE_REPAIR[0]),
                           DialogueAction::REPAIR_ITEM);
        } else {
            begin_dialogue("Momo", NPC_B_DIALOGUE_WAIT,
                           sizeof(NPC_B_DIALOGUE_WAIT) /
                               sizeof(NPC_B_DIALOGUE_WAIT[0]));
        }
    } else if(nearby_interaction_ == interaction::Target::OBJECT &&
              quest_state_ == QuestState::SEARCHING_ITEM) {
        begin_dialogue("Objet endommage", OBJECT_DIALOGUE,
                       sizeof(OBJECT_DIALOGUE) /
                           sizeof(OBJECT_DIALOGUE[0]),
                       DialogueAction::FIND_ITEM);
    }
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

void GameScene::update_dialogue_camera() {
    if(!player_ || !camera_) {
        return;
    }

    const auto target =
        player_->transform->position() + smlt::Vec3(0.0f, 1.0f, 0.0f);
    const auto orbit =
        smlt::Quaternion(smlt::Vec3::up(), smlt::Degrees(camera_yaw_));
    const float elevation =
        smlt::Degrees(camera_elevation_).to_radians().to_float();
    const auto direction = -orbit.forward() * std::cos(elevation) +
                           smlt::Vec3::up() * std::sin(elevation);

    camera_->transform->set_position(target + direction * CAMERA_DISTANCE);
    camera_->transform->look_at(target);
}

void GameScene::on_update(float dt) {
    if(input->axis_was_pressed(RESET)) {
        reset_player();
        return;
    }

    if(dialogue_open_) {
        if(input->axis_was_pressed(INTERACT)) {
            advance_dialogue();
        }
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
    player_height_ = ground_height(old_position.x, old_position.z,
                                   position.x, position.z, player_height_);
    player_->transform->set_position({position.x, player_height_, position.z});

    if(movement.length_squared() > 0.0001f) {
        const auto facing =
            smlt::Radians(std::atan2(-movement.x, -movement.z)).to_degrees();
        player_->transform->set_orientation(
            smlt::Quaternion(smlt::Vec3::up(), facing));
    }

    const bool interaction_changed = update_nearby_interaction();
    if(input->axis_was_pressed(INTERACT) &&
       nearby_interaction_ != interaction::Target::NONE) {
        interact_with_nearby();
    }
    if(interaction_changed) {
        update_ui();
    }
    if(!dialogue_open_) {
        update_camera();
    }
}

void GameScene::on_unload() {
    stop_music();

    // InputManager outlives this scene: remove only the bindings we created.
    for(const auto* name : {FORWARD, STRAFE, TURN, TILT, RESET,
                            TURN_LEFT, TURN_RIGHT, INTERACT}) {
        input->destroy_axises(name);
    }
    world_root_ = nullptr;
    ui_root_ = nullptr;
    player_ = nullptr;
    npc_a_ = nullptr;
    npc_b_ = nullptr;
    camera_ = nullptr;
    ui_camera_ = nullptr;
    interaction_label_ = nullptr;
    dialogue_name_ = nullptr;
    dialogue_text_ = nullptr;
    object_ = nullptr;
    victory_marker_ = nullptr;
    obstacles_.clear();
    camera_obstacles_.clear();
    platforms_.clear();
}
