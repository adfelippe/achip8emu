#pragma once

#include <cstdint>

class IDisplay {
public:
    IDisplay() {}
    virtual ~IDisplay() {}

    /** Creates and initializes a display window with size X,Y.
     * 
     * @param x_size horizontal size of the window in pixels
     * @param y_size vertical size of the window in pixels
     * 
     * Throws an exception is fails to initialize
     */
    virtual void initialize(uint32_t x_size, uint32_t y_size) {};

    /** Draws a fixed number of pixels at a given X,Y position on the screen
     * 
     * @param n_pixels to be drawn
     * @param x_pos horizontal position to start the drawing
     * @param y_pos vertical position to start the drawing
     * 
     * Throws an exception is fails to draw
     */
    virtual void draw(uint32_t n_pixels, uint32_t x_pos, uint32_t y_pos) {};
};
