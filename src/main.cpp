#include "chip8.hpp"
#include <iostream>
#include "SdlDisplay.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Failed to run: 2 arguments needed.\n";
        std::cout << "Help:\n";
        std::cout << "    ./achip8emu <file_path>\n";
        return EXIT_FAILURE;
    }
    
    std::shared_ptr<IDisplay> sdl_display = std::make_shared<SdlDisplay>();
    sdl_display->initialize(Chip8::kDisplaySizeX, Chip8::kDisplaySizeY);
    
    auto chip8 = Chip8(sdl_display);
    chip8.load(argv[1]);
    chip8.run();

    return EXIT_SUCCESS;
}
