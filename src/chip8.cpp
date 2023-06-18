#include "chip8.hpp"
#include <cstring>
#include <chrono>

Chip8::Chip8() {
    /*
     * Hex sprites (0 - F). Programs may use these sprites as their font.
     * However, in practice, most games implement their own font.
     * They must be stored in the first 512 bytes of the memory.
     */ 
    const uint8_t hex_sprites[] = {0xF0, 0x90, 0x90, 0x90, 0xF0,    // 0
                                   0x20, 0x60, 0x20, 0x20, 0x70,    // 1
                                   0xF0, 0x10, 0xF0, 0x80, 0xF0,    // 2
                                   0xF0, 0x10, 0xF0, 0x10, 0xF0,    // 3
                                   0x90, 0x90, 0xF0, 0x10, 0x10,    // 4
                                   0xF0, 0x80, 0xF0, 0x10, 0xF0,    // 5
                                   0xF0, 0x80, 0xF0, 0x90, 0xF0,    // 6
                                   0xF0, 0x10, 0x20, 0x40, 0x40,    // 7
                                   0xF0, 0x90, 0xF0, 0x90, 0xF0,    // 8
                                   0xF0, 0x90, 0xF0, 0x10, 0xF0,    // 9
                                   0xF0, 0x90, 0xF0, 0x90, 0x90,    // A
                                   0xE0, 0x90, 0xE0, 0x90, 0xE0,    // B
                                   0xF0, 0x80, 0x80, 0x80, 0xF0,    // C
                                   0xE0, 0x90, 0x90, 0x90, 0xE0,    // D
                                   0xF0, 0x80, 0xF0, 0x80, 0xF0,    // E
                                   0xF0, 0x80, 0xF0, 0x80, 0x80};   // F
    
    memcpy(&memory_[kSpritesMemLocation], hex_sprites, sizeof(hex_sprites));
}

void Chip8::load(const std::string &path) {}

void Chip8::run(void) {}

void Chip8::runDelayTimer(void) {
    static auto last_timestamp = std::chrono::steady_clock::now();

    if (reg_.DT) {
        const auto elapsed_time = std::chrono::steady_clock::now() - last_timestamp;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() >= kTimersDecrementRateMs) {
            --reg_.DT;
        }
    }

    last_timestamp = std::chrono::steady_clock::now();
}

void Chip8::runSoundTimer(void) {
    static auto last_timestamp = std::chrono::steady_clock::now();

    if (reg_.ST) {
        buzzerOn();
        const auto elapsed_time = std::chrono::steady_clock::now() - last_timestamp;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() >= kTimersDecrementRateMs) {
            --reg_.ST;
        }
    } else {
        buzzerOff();
    }

    last_timestamp = std::chrono::steady_clock::now();
}

void Chip8::buzzerOn(void) {}

void Chip8::buzzerOff(void) {}

