#pragma once

#include <cstdint>
#include <exception>

class IDisplay {
public:
    
    IDisplay() {}
    virtual ~IDisplay() {}

    /** Draws a fixed number of pixels at a given X,Y position on the screen
     * 
     * @param n_pixels to be drawn
     * @param x_pos horizontal position to start the drawing
     * @param y_pos vertical position to start the drawing
     * 
     * Throws an exception is fails to draw
     */
    virtual void draw(uint32_t x_pos, uint32_t y_pos) {};

    /** Renders a previous draw() on the screen
     * 
     */
    virtual void render(uint8_t *screen_buffer) {};

    /** Clears the entire screen
     * 
     */
    virtual void clear(void) {};
};
