#include "IDisplay.hpp"

class SdlDisplay : public IDisplay {
public:
    SdlDisplay() {}
    virtual ~SdlDisplay() {}

    void initialize(uint32_t x_size, uint32_t y_size) override;
    void draw(uint32_t n_pixels, uint32_t x_pos, uint32_t y_pos) override;
};
