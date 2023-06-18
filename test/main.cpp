#include "chip8.hpp"

int main(void) {
    auto chip8 = Chip8();
    chip8.load("./2-ibm-logo.ch8");
    chip8.run();
    return 0;
}
