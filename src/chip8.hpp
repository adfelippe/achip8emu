#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Chip8 {
public:
    Chip8();
    virtual ~Chip8() {}

    void load(const std::string &path);
    void run(void);

private:
    void runDelayTimer(void);
    void runSoundTimer(void);
    void buzzerOn(void);
    void buzzerOff(void);

    // CHIP-8 Registers
    struct Register {
        // General purpose registers (V0 - VF)
        uint8_t V[16];
        // 12-bit I register (usually to point at locations in memory)
        uint16_t I;
        // Program Counter
        uint16_t PC;
        // Stack Pointer
        uint8_t SP;
        // Delay Timer
        uint8_t DT;
        // Sound Timer
        uint8_t ST;
        // Stack region
        uint16_t stack[16];
    };

    // CHIP-8 display size if 64 x 32 pixels
    static constexpr size_t kDisplaySize = 64 * 32;
    // CHIP-8 memory is 4 KB
    static constexpr size_t kMemorySize = 4096;
    static constexpr uint8_t kSpritesMemLocation = 0x00;
    // 60 Hz = 16.666667 ms (period)
    static constexpr double kTimersDecrementRateMs = 16.666667F;

    Register reg_;
    uint8_t memory_[kMemorySize];
};
        