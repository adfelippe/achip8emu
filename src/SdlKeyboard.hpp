#pragma once

#include "IKeyboard.hpp"

#include <exception>
#include <string>
#include <SDL2/SDL.h>


class SdlKeyboard : public IKeyboard {
public:
    SdlKeyboard() {}
    virtual ~SdlKeyboard() {}

    IKeyboard::Key getKey(void) override;

private:
    char convertScanCodeToCharValue(SDL_Scancode scancode);
};
