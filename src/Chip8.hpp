#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <stack>

#include "IDisplay.hpp"

class Chip8 {
public:
    class Chip8Exception : public std::exception {
    public:
        Chip8Exception(const std::string &err_msg) : err_msg_(err_msg) {} 
        const char* what() const throw() { return err_msg_.c_str(); }
    private:
        const std::string err_msg_;
    };

    Chip8(size_t memory_start_offset, const std::shared_ptr<IDisplay> &display);
    // Default memory start offset used (0x200)
    Chip8(const std::shared_ptr<IDisplay> &display);
    virtual ~Chip8() {}

    void load(const std::string &path);
    void run(void);
    static size_t displaySize(void);

    enum Opcodes {
        kClearScreen = 0x00E0,
        kReturn = 0x00EE,
        kJump = 0x01,
        kCall = 0x02,
        kSkipEqual = 0x03,
        kSetVxReg = 0x06,
        kAddValueToVxReg = 0x07,
        kVRegOperation = 0x08,
        kSkipNotEqual = 0x09,
        kSetIndexRegI = 0x0A,
        kJumpToAddrPlusV0 = 0x0B,
        kSetRandom = 0x0C,
        kDisplayDraw = 0x0D,
        kSkipNetIfKey = 0x0E,
        kMisc = 0x0F,
        // Sub-opcodes for opcodes started in 0x0F
        kMiscDelayTimerValue = 0x07,
        kMiscWaitForKey = 0x0A,
        kMiscSetDelayTimer = 0x15,
        kMiscSetSoundTimer = 0x18,
        kMiscAddToIndex = 0x1E,
        kMiscFontChar = 0x29,
        kMiscStoreBcd = 0x33,
        kMiscStoreMemory = 0x55,
        kMiscLoadMemory = 0x65
    };

    struct Key {
        enum class State {
            kPressed = 0x01,
            kReleased = 0x00
        };

        uint8_t key;
        State state;
    };

    // CHIP-8 display size if 64 x 32 pixels
    static constexpr size_t kDisplayWidth = 64;
    static constexpr size_t kDisplayHeight = 32;

private:
    void runDelayTimer(void);
    void runSoundTimer(void);
    void buzzerOn(void);
    void buzzerOff(void);
    uint16_t fetchInstruction(void);
    void decodeInstruction(uint16_t opcode);
    // Instructions
    void jump(uint16_t address);
    void callSubroutine(uint16_t address);
    void skipIfEqual(uint8_t v_reg, uint8_t value);
    void setVxRegister(uint8_t v_reg, uint8_t value);
    void addValueToVxRegister(uint8_t v_reg, uint8_t value);
    void setIndexRegister(uint16_t value);
    void displayDraw(uint8_t x, uint8_t y, uint8_t n);
    void clearScreen(void);
    void returnFromSubroutine(void);
    void runVRegOperation(uint8_t x, uint8_t y, uint8_t n);
    void skipNext(uint8_t x, uint8_t y);
    void jumpToAddrPlusV0(uint16_t address);
    void setRandomByteToVx(uint8_t v_reg, uint8_t value);
    void skipNetIfKey(uint8_t x, uint8_t y, uint8_t n);
    void decodeMisc(uint8_t x, uint8_t y, uint8_t n);
    bool keyIsPressed(uint8_t x);
    Chip8::Key getKey(void);

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
        std::stack<uint16_t> stack;
    };

    // CHIP-8 memory is 4 KB
    static constexpr size_t kMemorySize = 4096;
    static constexpr size_t kMemoryStartOffsetDefault = 0x200;
    static constexpr uint8_t kSpritesMemLocation = 0x00;
    // 60 Hz = 16667 us (period)
    static constexpr int64_t kCpuPeriodUs = 16667;

    Register reg_;
    uint8_t memory_[kMemorySize];
    uint8_t screen_buffer_[kDisplayWidth * kDisplayHeight];
    size_t memory_start_offset_;
    std::shared_ptr<IDisplay> display_;
};
        