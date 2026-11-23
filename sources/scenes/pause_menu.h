#pragma once

#include <algorithm>

namespace pause_menu {

enum class Item { RESUME, MUSIC, RESTART };
enum class Action { NONE, RESUME, RESTART, VOLUME_CHANGED };

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
            open_ = false;
            return Action::RESUME;
        }
        if(input.up != input.down) {
            const int direction = input.down ? 1 : -1;
            selected_ = static_cast<Item>((static_cast<int>(selected_) +
                                           direction + 3) % 3);
            return Action::NONE;
        }
        if(selected_ == Item::MUSIC && input.left != input.right) {
            const int previous = music_volume_;
            music_volume_ = std::clamp(music_volume_ + (input.right ? 10 : -10),
                                       0, 100);
            return previous != music_volume_
                ? Action::VOLUME_CHANGED : Action::NONE;
        }
        if(input.confirm && selected_ != Item::MUSIC) {
            open_ = false;
            return selected_ == Item::RESTART ? Action::RESTART : Action::RESUME;
        }
        return Action::NONE;
    }

private:
    bool open_ = false;
    Item selected_ = Item::RESUME;
    int music_volume_ = 50;
};

} // namespace pause_menu
