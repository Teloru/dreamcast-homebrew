#pragma once

#include <cstddef>

#include "pause_menu.h"

namespace title_menu {

enum class Screen { PROMPT, MAIN, OPTIONS, PLAYING };
enum class Action { NONE, START_GAME, VOLUME_DOWN, VOLUME_UP, BACK };

class State {
public:
    Screen screen() const { return screen_; }
    bool is_active() const { return screen_ != Screen::PLAYING; }
    std::size_t selected() const { return selected_; }

    void show_prompt() {
        screen_ = Screen::PROMPT;
        selected_ = 0;
    }

    // Share controller press data with pause; volume itself has a single owner.
    Action update(const pause_menu::Input& input) {
        if(screen_ == Screen::PLAYING) {
            return Action::NONE;
        }
        if(screen_ == Screen::PROMPT) {
            if(input.toggle) {
                screen_ = Screen::MAIN;
                selected_ = 0;
            }
            return Action::NONE;
        }
        // Start opens the prompt's menu only; it cannot also launch a game.
        if(input.toggle) {
            return Action::NONE;
        }
        if(input.cancel) {
            if(screen_ == Screen::OPTIONS) {
                screen_ = Screen::MAIN;
                selected_ = 1;
            } else {
                show_prompt();
            }
            return Action::BACK;
        }
        if(input.up != input.down) {
            selected_ = 1 - selected_;
            return Action::NONE;
        }
        if(screen_ == Screen::OPTIONS && selected_ == 0 &&
           input.left != input.right) {
            return input.right ? Action::VOLUME_UP : Action::VOLUME_DOWN;
        }
        if(!input.confirm) {
            return Action::NONE;
        }
        if(screen_ == Screen::MAIN) {
            if(selected_ == 0) {
                screen_ = Screen::PLAYING;
                return Action::START_GAME;
            }
            screen_ = Screen::OPTIONS;
            selected_ = 0;
        } else if(selected_ == 1) {
            screen_ = Screen::MAIN;
            selected_ = 1;
            return Action::BACK;
        }
        return Action::NONE;
    }

private:
    Screen screen_ = Screen::PROMPT;
    std::size_t selected_ = 0;
};

} // namespace title_menu
