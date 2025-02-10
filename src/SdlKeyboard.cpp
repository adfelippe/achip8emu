#include "SdlKeyboard.hpp"

IKeyboard::Key SdlKeyboard::getKey(void) {
    auto key = IKeyboard::Key();
    SDL_Event event;

    if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN) {
        auto scancode = event.key.keysym.scancode;
        key.state = Key::State::kPressed;
        key.value = convertScanCodeToCharValue(scancode);

    } else {
        key.state = Key::State::kReleased;
    }

    return key;
}

char SdlKeyboard::convertScanCodeToCharValue(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_A: return 'a';
        case SDL_SCANCODE_B: return 'b';
        case SDL_SCANCODE_C: return 'c';
        case SDL_SCANCODE_D: return 'd';
        case SDL_SCANCODE_E: return 'e';
        case SDL_SCANCODE_F: return 'f';
        case SDL_SCANCODE_G: return 'g';
        case SDL_SCANCODE_H: return 'h';
        case SDL_SCANCODE_I: return 'i';
        case SDL_SCANCODE_J: return 'j';
        case SDL_SCANCODE_K: return 'k';
        case SDL_SCANCODE_L: return 'l';
        case SDL_SCANCODE_M: return 'm';
        case SDL_SCANCODE_N: return 'n';
        case SDL_SCANCODE_O: return 'o';
        case SDL_SCANCODE_P: return 'p';
        case SDL_SCANCODE_Q: return 'q';
        case SDL_SCANCODE_R: return 'r';
        case SDL_SCANCODE_S: return 's';
        case SDL_SCANCODE_T: return 't';
        case SDL_SCANCODE_U: return 'u';
        case SDL_SCANCODE_V: return 'v';
        case SDL_SCANCODE_W: return 'w';
        case SDL_SCANCODE_X: return 'x';
        case SDL_SCANCODE_Y: return 'y';
        case SDL_SCANCODE_Z: return 'z';
        case SDL_SCANCODE_0: return '0';
        case SDL_SCANCODE_1: return '1';
        case SDL_SCANCODE_2: return '2';
        case SDL_SCANCODE_3: return '3';
        case SDL_SCANCODE_4: return '4';
        case SDL_SCANCODE_5: return '5';
        case SDL_SCANCODE_6: return '6';
        case SDL_SCANCODE_7: return '7';
        case SDL_SCANCODE_8: return '8';
        case SDL_SCANCODE_9: return '9';
        default: return '\0';
    }
}
