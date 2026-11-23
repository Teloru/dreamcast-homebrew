#include "../sources/scenes/pause_menu.h"

#include <cassert>
#include <iostream>

namespace {

using pause_menu::Action;
using pause_menu::Input;
using pause_menu::Item;
using pause_menu::State;

Input press(bool Input::*button) {
    Input input;
    input.*button = true;
    return input;
}

void test_closed_menu_ignores_gameplay_buttons() {
    State menu;
    assert(!menu.is_open());
    assert(menu.music_volume() == 50);
    for(auto button : {&Input::confirm, &Input::cancel, &Input::up,
                        &Input::down, &Input::left, &Input::right}) {
        assert(menu.update(press(button)) == Action::NONE);
        assert(!menu.is_open());
        assert(menu.selected() == Item::RESUME);
        assert(menu.music_volume() == 50);
    }
}

void test_navigation_wraps_and_confirm_has_one_action() {
    State menu;
    menu.update(press(&Input::toggle));
    assert(menu.is_open());
    assert(menu.selected() == Item::RESUME);
    menu.update(press(&Input::up));
    assert(menu.selected() == Item::RESTART);
    assert(menu.update(press(&Input::confirm)) == Action::RESTART);
    assert(!menu.is_open());
    assert(menu.update(press(&Input::confirm)) == Action::NONE);

    menu.update(press(&Input::toggle));
    assert(menu.selected() == Item::RESUME);
    menu.update(press(&Input::down));
    assert(menu.selected() == Item::MUSIC);
    assert(menu.update(press(&Input::confirm)) == Action::NONE);
    assert(menu.is_open());
    menu.update(press(&Input::down));
    assert(menu.selected() == Item::RESTART);
    menu.update(press(&Input::down));
    assert(menu.selected() == Item::RESUME);
    assert(menu.update(press(&Input::confirm)) == Action::RESUME);
    assert(!menu.is_open());
}

void test_volume_limits_and_session_lifetime() {
    State menu;
    menu.update(press(&Input::toggle));
    assert(menu.update(press(&Input::left)) == Action::NONE);
    assert(menu.music_volume() == 50);
    menu.update(press(&Input::down));
    for(int volume = 40; volume >= 0; volume -= 10) {
        assert(menu.update(press(&Input::left)) == Action::VOLUME_CHANGED);
        assert(menu.music_volume() == volume);
    }
    assert(menu.update(press(&Input::left)) == Action::NONE);
    assert(menu.music_volume() == 0);
    for(int volume = 10; volume <= 100; volume += 10) {
        assert(menu.update(press(&Input::right)) == Action::VOLUME_CHANGED);
        assert(menu.music_volume() == volume);
    }
    assert(menu.update(press(&Input::right)) == Action::NONE);
    assert(menu.music_volume() == 100);
    assert(menu.update(press(&Input::cancel)) == Action::RESUME);
    menu.update(press(&Input::toggle));
    assert(menu.selected() == Item::RESUME);
    assert(menu.music_volume() == 100);
    menu.update(press(&Input::up));
    assert(menu.update(press(&Input::confirm)) == Action::RESTART);
    menu.update(press(&Input::toggle));
    assert(menu.music_volume() == 100);
}

void test_simultaneous_presses_do_not_trigger_an_unintended_restart() {
    State menu;
    Input input;
    input.toggle = input.up = input.confirm = true;
    assert(menu.update(input) == Action::NONE);
    assert(menu.is_open());
    assert(menu.selected() == Item::RESUME);

    input = {};
    input.up = input.confirm = true;
    assert(menu.update(input) == Action::NONE);
    assert(menu.selected() == Item::RESTART);
    assert(menu.is_open());

    input = {};
    input.cancel = input.confirm = true;
    assert(menu.update(input) == Action::RESUME);
    assert(!menu.is_open());

    menu.update(press(&Input::toggle));
    menu.update(press(&Input::up));
    input = {};
    input.toggle = input.confirm = true;
    assert(menu.update(input) == Action::RESUME);
    assert(!menu.is_open());

    menu.update(press(&Input::toggle));
    menu.update(press(&Input::down));
    input = {};
    input.up = input.down = input.left = input.right = true;
    assert(menu.update(input) == Action::NONE);
    assert(menu.selected() == Item::MUSIC);
    assert(menu.music_volume() == 50);
}

} // namespace

int main() {
    test_closed_menu_ignores_gameplay_buttons();
    test_navigation_wraps_and_confirm_has_one_action();
    test_volume_limits_and_session_lifetime();
    test_simultaneous_presses_do_not_trigger_an_unintended_restart();
    std::cout << "pause menu tests passed\n";
}
