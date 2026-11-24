#pragma once

#include <algorithm>

namespace pause_menu {

enum class Item { RESUME, MUSIC, RESTART, RETURN_TO_TITLE, COUNT };
enum class Action { NONE, RESUME, RESTART, RETURN_TO_TITLE, VOLUME_CHANGED };

struct Input {
    bool toggle = false;
    bool cancel = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool confirm = false;
};

class State {
public:
    bool is_open() const { return open_; }
    Item selected() const { return selected_; }
    int music_volume() const { return music_volume_; }

    void close() {
        open_ = false;
        selected_ = Item::RESUME;
    }

    // Shared with the title options; direction chooses one 10% step.
    bool change_music_volume(int direction) {
        const int previous = music_volume_;
        const int step = (direction > 0) - (direction < 0);
        music_volume_ = std::clamp(music_volume_ + step * 10, 0, 100);
        return previous != music_volume_;
    }

    // Inputs are presses, not held values: one step per press.
    Action update(const Input& input) {
        if(input.toggle) {
            open_ = !open_;
            selected_ = Item::RESUME;
            return open_ ? Action::NONE : Action::RESUME;
        }
        if(!open_) {
            return Action::NONE;
        }
        if(input.cancel) {
            close();
            return Action::RESUME;
        }
        if(input.up != input.down) {
            const int direction = input.down ? 1 : -1;
            constexpr int count = static_cast<int>(Item::COUNT);
            selected_ = static_cast<Item>((static_cast<int>(selected_) +
                                           direction + count) % count);
            return Action::NONE;
        }
        if(selected_ == Item::MUSIC && input.left != input.right) {
            return change_music_volume(input.right ? 1 : -1)
                ? Action::VOLUME_CHANGED : Action::NONE;
        }
        if(input.confirm && selected_ != Item::MUSIC) {
            const auto selection = selected_;
            close();
            if(selection == Item::RETURN_TO_TITLE) {
                return Action::RETURN_TO_TITLE;
            }
            return selection == Item::RESTART ? Action::RESTART : Action::RESUME;
        }
        return Action::NONE;
    }

private:
    bool open_ = false;
    Item selected_ = Item::RESUME;
    int music_volume_ = 50;
};

} // namespace pause_menu
