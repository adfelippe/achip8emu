#pragma once

#include <cstdint>
#include <exception>

class IDisplay {
public:
    IDisplay() {}
    virtual ~IDisplay() {}

    /** Creates and initializes a display window with size X,Y.
     * 
     * @param display_width horizontal size of the window in pixels
     * @param display_height vertical size of the window in pixels
     * 
     * Throws an exception is fails to initialize
     */
    virtual void initialize(uint32_t display_width, uint32_t display_height) {};

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
