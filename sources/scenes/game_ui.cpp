#include "game.h"

#include <cstdint>
#include <string>

namespace {

inline void set_utf8_text(smlt::ui::Label* label, const std::string& text);

inline void set_utf8_text(smlt::ui::Label* label, const char* text) {
    set_utf8_text(label, std::string(text ? text : ""));
}

inline void set_utf8_text(smlt::ui::Label* label, const std::string& text) {
    if(label) {
        std::u16string decoded;
        decoded.reserve(text.size());
        for(std::size_t i = 0; i < text.size();) {
            const auto first = static_cast<unsigned char>(text[i]);
            uint32_t codepoint = 0;
            std::size_t length = 0;

            if(first < 0x80) {
                codepoint = first;
                length = 1;
            } else if((first & 0xE0) == 0xC0 && i + 1 < text.size()) {
                codepoint = ((first & 0x1F) << 6) |
                            (static_cast<unsigned char>(text[i + 1]) & 0x3F);
                length = 2;
            } else if((first & 0xF0) == 0xE0 && i + 2 < text.size()) {
                codepoint = ((first & 0x0F) << 12) |
                            ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) |
                            (static_cast<unsigned char>(text[i + 2]) & 0x3F);
                length = 3;
            } else if((first & 0xF8) == 0xF0 && i + 3 < text.size()) {
                codepoint = ((first & 0x07) << 18) |
                            ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12) |
                            ((static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6) |
                            (static_cast<unsigned char>(text[i + 3]) & 0x3F);
                length = 4;
            } else {
                codepoint = 0xFFFD;
                length = 1;
            }

            if(codepoint <= 0xFFFF) {
                decoded.push_back(static_cast<char16_t>(codepoint));
            } else if(codepoint <= 0x10FFFF) {
                codepoint -= 0x10000;
                decoded.push_back(static_cast<char16_t>(0xD800 + (codepoint >> 10)));
                decoded.push_back(static_cast<char16_t>(0xDC00 + (codepoint & 0x3FF)));
            } else {
                decoded.push_back(static_cast<char16_t>(0xFFFD));
            }
            i += length;
        }
        label->set_text(::unicode(decoded.begin(), decoded.end()));
    }
}

} // namespace

void GameScene::setup_ui() {
    ui_root_ = create_child<smlt::Stage>();
    ui_camera_ = create_child<smlt::Camera2D>();
    ui_camera_->set_orthographic_projection(0.0f, 640.0f, 0.0f, 480.0f);
    ui_camera_->transform->set_position({0.0f, 0.0f, 10.0f});
    auto ui_layer = compositor->create_layer(ui_root_, ui_camera_, 1);
    ui_layer->set_clear_flags(0);
    smlt::Params label_params;
    label_params.set("text", std::string(""));
    interaction_label_ = ui_root_->create_child<smlt::ui::Label>(label_params);
    interaction_label_->resize(smlt::ui::Px(380), smlt::ui::Px(36));
    interaction_label_->transform->set_position({130.0f, 430.0f, 0.0f});
    interaction_label_->set_text_alignment(smlt::ui::TEXT_ALIGNMENT_CENTER);
    interaction_label_->set_text_color(smlt::Color::white());
    interaction_label_->set_border_width(smlt::ui::Px(1));
    interaction_label_->set_border_color(smlt::Color(0.9f, 0.9f, 0.9f, 1.0f));
    interaction_label_->set_background_color(smlt::Color::none());
    set_utf8_text(interaction_label_, "");
    interaction_label_->set_opacity(0.0f);
    interaction_label_->set_precedence(10);

    smlt::Params name_params;
    name_params.set("text", std::string(""));
    dialogue_name_ = ui_root_->create_child<smlt::ui::Label>(name_params);
    dialogue_name_->resize(smlt::ui::Px(560), smlt::ui::Px(32));
    dialogue_name_->transform->set_position({40.0f, 148.0f, 0.0f});
    dialogue_name_->set_text_alignment(smlt::ui::TEXT_ALIGNMENT_LEFT);
    dialogue_name_->set_text_color(smlt::Color(1.0f, 0.85f, 0.35f, 1.0f));
    dialogue_name_->set_border_width(smlt::ui::Px(1));
    dialogue_name_->set_border_color(smlt::Color(0.9f, 0.9f, 0.9f, 1.0f));
    dialogue_name_->set_background_color(smlt::Color::none());
    dialogue_name_->set_font("Orbitron", smlt::ui::Px(18));
    set_utf8_text(dialogue_name_, "");
    dialogue_name_->set_opacity(0.0f);
    dialogue_name_->set_precedence(20);

    smlt::Params text_params;
    text_params.set("text", std::string(""));
    dialogue_text_ = ui_root_->create_child<smlt::ui::Label>(text_params);
    dialogue_text_->resize(smlt::ui::Px(560), smlt::ui::Px(120));
    dialogue_text_->transform->set_position({40.0f, 16.0f, 0.0f});
    dialogue_text_->set_text_alignment(smlt::ui::TEXT_ALIGNMENT_LEFT);
    dialogue_text_->set_wrap_mode(smlt::ui::WRAP_MODE_WORD);
    dialogue_text_->set_text_color(smlt::Color::white());
    dialogue_text_->set_border_width(smlt::ui::Px(1));
    dialogue_text_->set_border_color(smlt::Color(0.9f, 0.9f, 0.9f, 1.0f));
    dialogue_text_->set_background_color(smlt::Color::none());
    dialogue_text_->set_font("Orbitron", smlt::ui::Px(18));
    set_utf8_text(dialogue_text_, "");
    dialogue_text_->set_opacity(0.0f);
    dialogue_text_->set_precedence(20);
    setup_menu();
    setup_title();
}

void GameScene::update_ui() {
    if(dialogue_open_) {
        const std::string name = dialogue_name_text_;
        const std::string text = dialogue_lines_ ? dialogue_lines_[dialogue_index_] : "";
        if(ui_dialogue_name_ != name) {
            ui_dialogue_name_ = name;
            set_utf8_text(dialogue_name_, name);
        }
        if(ui_dialogue_text_ != text) {
            ui_dialogue_text_ = text;
            set_utf8_text(dialogue_text_, text);
        }
    } else {
        if(!ui_dialogue_name_.empty()) {
            ui_dialogue_name_.clear();
            set_utf8_text(dialogue_name_, "");
        }
        if(!ui_dialogue_text_.empty()) {
            ui_dialogue_text_.clear();
            set_utf8_text(dialogue_text_, "");
        }
    }

    const bool show_dialogue = dialogue_open_ && !menu_.is_open() && !title_.is_active();
    if(dialogue_ui_visible_ != show_dialogue) {
        dialogue_ui_visible_ = show_dialogue;
        const auto opacity = dialogue_ui_visible_ ? 1.0f : 0.0f;
        const auto color = dialogue_ui_visible_
                               ? smlt::Color(0.12f, 0.18f, 0.28f, 0.95f)
                               : smlt::Color::none();
        if(dialogue_name_) {
            dialogue_name_->set_opacity(opacity);
            dialogue_name_->set_background_color(color);
        }
        if(dialogue_text_) {
            dialogue_text_->set_opacity(opacity);
            dialogue_text_->set_background_color(color);
        }
    }

    std::string interaction;
    if(dialogue_open_) {
        interaction = "A : suite";
    } else if(nearby_interaction_ != ::interaction::Target::NONE) {
        interaction = "A : parler / recuperer";
    } else if(quest_state_ == QuestState::SEARCHING_ITEM) {
        interaction = "Objectif : trouver la piece";
    } else if(quest_state_ == QuestState::ITEM_FOUND) {
        interaction = "Objectif : voir Momo";
    } else if(quest_state_ == QuestState::ITEM_REPAIRED) {
        interaction = "Objectif : retourner voir Mia";
    } else if(quest_state_ == QuestState::COMPLETED) {
        interaction = "Quete terminee";
    }
    if(ui_interaction_text_ != interaction) {
        ui_interaction_text_ = interaction;
        set_utf8_text(interaction_label_, interaction);
    }
    const bool interaction_visible = !interaction.empty() && !menu_.is_open() &&
                                     !title_.is_active();
    if(interaction_ui_visible_ != interaction_visible) {
        interaction_ui_visible_ = interaction_visible;
        if(interaction_label_) {
            interaction_label_->set_opacity(interaction_visible ? 1.0f : 0.0f);
            interaction_label_->set_background_color(
                interaction_visible
                    ? smlt::Color(0.05f, 0.05f, 0.05f, 0.85f)
                    : smlt::Color::none());
        }
    }
}

