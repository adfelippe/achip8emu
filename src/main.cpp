#include "Chip8.hpp"
#include <iostream>
#include "SdlDisplay.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Failed to run: 2 arguments needed.\n";
        std::cout << "Help:\n";
        std::cout << "    ./achip8emu <file_path>\n";
        return EXIT_FAILURE;
    }
    
    std::shared_ptr<IDisplay> sdl_display = std::make_shared<SdlDisplay>(Chip8::kDisplayWidth, Chip8::kDisplayHeight);
    auto chip8 = Chip8(sdl_display);
    try {
        chip8.load(argv[1]);
    } catch (const std::exception &err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        return EXIT_FAILURE;
    }
    chip8.run();

    return EXIT_SUCCESS;
}
