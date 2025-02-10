#pragma once

class IKeyboard {
public:
    IKeyboard() {}
    virtual ~IKeyboard() {}

    struct Key {
        enum class State {
            kReleased = 1,
            kPressed = 0
        };

        State state;
        char value;
    };

    virtual Key getKey(void) = 0;
    virtual bool quitClicked(void) = 0;
};
