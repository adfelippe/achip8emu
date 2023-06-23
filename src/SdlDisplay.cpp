#include "SdlDisplay.hpp"
#include <iostream>
#include <memory>

SdlDisplay::SdlDisplay() : window_(nullptr), renderer_(nullptr), display_width_(0), display_height_(0) {}

SdlDisplay::~SdlDisplay() {
    if (window_) {
        SDL_DestroyWindow(window_);
    }

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    SDL_Quit();
}

void SdlDisplay::initialize(uint32_t display_width, uint32_t display_height) {
    if (!display_width || !display_height) {
        throw SdlDisplayException("Failed to initialize SDL: screen size invalid  (" + std::to_string(display_width_) + 
                                  ", " + std::to_string(display_height_) + ")");
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw SdlDisplayException("Failed to initialize SDL lib: " + std::string(SDL_GetError()));
    }

    display_width_ = display_width;
    display_height_ = display_height;

    window_ = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                               display_width_ * kScaleFactor, display_height_ * kScaleFactor, 0);
    if (!window_) {
        throw SdlDisplayException("Failed to create SDL window: " + std::string(SDL_GetError()));
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        throw SdlDisplayException("Failed to create SDL renderer: " + std::string(SDL_GetError()));
    }
}

void SdlDisplay::draw(uint32_t x_pos, uint32_t y_pos) {
    if (!window_) { 
        throw SdlDisplayException("Failed to draw: window not initialized");
    }

    if (!renderer_) { 
        throw SdlDisplayException("Failed to draw: renderer not initialized");
    }
}

void SdlDisplay::render(uint8_t *screen_buffer) {
    if (!window_) { 
        throw SdlDisplayException("Failed to render: window not initialized");
    }

    if (!renderer_) { 
        throw SdlDisplayException("Failed to render: renderer not initialized");
    }

    if (!display_width_ || !display_height_) {
        throw SdlDisplayException("Failed to render: screen size invalid -> " + std::to_string(display_width_) + ", " +
                                  std::to_string(display_height_));
    }

    auto display_size = display_width_ * display_height_;
    SDL_Rect rectangle = {.x = 0, .y = 0, .w = kScaleFactor, .h = kScaleFactor};

    for (uint32_t i = 0; i < display_size; ++i) {
        // Create a rectangle to represent a scaled pixel
        rectangle.x = (i % display_width_) * kScaleFactor;
        rectangle.y = (i / display_width_) * kScaleFactor;

        if (screen_buffer[i]) {
            SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer_, &rectangle);
        } else {
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer_, &rectangle);
        }
    }

    SDL_RenderPresent(renderer_);
}

void SdlDisplay::clear(void) {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);
}
