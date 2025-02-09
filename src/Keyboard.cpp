#include "Keyboard.hpp"
#include <ncurses.h>

Keyboard::Keyboard() {}

Keyboard::~Keyboard() {}

IKeyboard::Key Keyboard::getKey(void) {
    auto key = IKeyboard::Key();
    /* Turn off getch() blocking and echo */
    nodelay(stdscr, TRUE);
    noecho();

    char c = getch();

    if (c == ERR) {
        key.state = Key::State::kReleased;
    } else {
        key.state = Key::State::kPressed;
        key.value = c;
        ungetch(c);
    }

    /* Restore blocking and echo */
    echo();
    nodelay(stdscr, FALSE);

    return key;
}
