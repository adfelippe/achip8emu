#pragma once

#include "IKeyboard.hpp"

#include <thread>
#include <memory>
#include <queue>
#include <SDL2/SDL.h>


class SdlKeyboard : public IKeyboard {
public:
    SdlKeyboard();
    virtual ~SdlKeyboard();

    IKeyboard::Key getKey(void) override;
    bool quitClicked(void) override;

private:
    char convertScanCodeToCharValue(SDL_Scancode scancode);
    void processEvents(void);
    
    bool runProccessEventsThread_;
    bool quitClicked_;
    std::thread eventsThread_;
    std::queue<IKeyboard::Key> keysQueue_;
};
