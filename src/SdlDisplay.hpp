#include "IDisplay.hpp"

#include <exception>
#include <string>
#include <SDL2/SDL.h>

class SdlDisplay : public IDisplay {
public:
    SdlDisplay();
    virtual ~SdlDisplay();

    class SdlDisplayException : public std::exception {
    public:
        SdlDisplayException(const std::string &err_msg) : err_msg_(err_msg) {} 
        const char* what() const throw() { return err_msg_.c_str(); }
    private:
        const std::string err_msg_;
    };

    void initialize(uint32_t display_width, uint32_t display_height) override;
    void draw(uint32_t x_pos, uint32_t y_pos) override;
    void render(uint8_t *screen_buffer) override;
    void clear(void) override;

private:
    static constexpr uint32_t kScaleFactor = 20;

    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture* texture_;
    uint32_t display_width_;
    uint32_t display_height_;
};

