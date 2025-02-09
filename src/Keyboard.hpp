#pragma once

#include "IKeyboard.hpp"

class Keyboard : public IKeyboard {
public:
    Keyboard();
    virtual ~Keyboard();

    IKeyboard::Key getKey(void) override;
};
