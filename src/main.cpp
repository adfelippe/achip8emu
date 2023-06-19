#include "chip8.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Failed to run: 2 arguments needed.\n";
        std::cout << "Help:\n";
        std::cout << "    ./achip8emu <file_path>\n";
    }
    
    auto chip8 = Chip8();
    chip8.load(argv[1]);
    chip8.run();
    return 0;
}
