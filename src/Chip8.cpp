#include "Chip8.hpp"

#include <cstring>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <thread>
#include <SDL2/SDL.h>

using namespace std::chrono_literals;

Chip8::Chip8(size_t memory_start_offset, const std::shared_ptr<IDisplay> &display) : 
    memory_start_offset_(memory_start_offset),
    display_(display) {
    std::memset(memory_, 0, kMemorySize * sizeof(uint8_t));
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
    // Data initialization                                   
    std::memcpy(&memory_[kSpritesMemLocation], hex_sprites, sizeof(hex_sprites));
    std::memset(screen_buffer_, 0, sizeof(screen_buffer_));
    std::memset(&reg_.V[0], 0, sizeof(reg_.V));
    reg_.I = 0;
    reg_.PC = 0;
    reg_.SP = 0;
    reg_.DT = 0;
    reg_.ST = 0;
}

Chip8::Chip8(const std::shared_ptr<IDisplay> &display) : Chip8(kMemoryStartOffsetDefault, display) {}

void Chip8::load(const std::string &path) {
    std::cout << "Loading " << path << "\n";

    std::ifstream f;
    f.open(path.c_str(), (std::ios::in | std::ios::binary));

    if (!f.good()) {
        throw Chip8Exception("Failed to load file " + path);
    }

    // TODO: Add check to avoid a program larger than memory to cause overflow
    std::filesystem::path file_path = path;
    auto file_size = std::filesystem::file_size(file_path);
    reg_.PC = memory_start_offset_;
    f.read((char*)&memory_[memory_start_offset_], file_size);
    std::cout << file_size << " bytes loaded successfully\n";
    f.close();
}

void Chip8::run(void) {
    auto start_time = std::chrono::steady_clock::now();

    while (1) {
        auto start_delay = std::chrono::steady_clock::now();
        auto opcode = fetchInstruction();
        decodeInstruction(opcode);

        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time).count() > 6000) {
            break;
        }

        // 60 Hz refresh rate
        std::this_thread::sleep_until(start_delay + 17ms);
    }
}

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

uint16_t Chip8::fetchInstruction(void) {
    // CHIP-8 opcodes are stored in big-endian
    uint16_t opcode = ((static_cast<uint16_t>(memory_[reg_.PC]) << 8) & 0xFF00) + memory_[reg_.PC + 1];
    reg_.PC += 2;
    return opcode;
}

void Chip8::decodeInstruction(uint16_t opcode) {
    std::cout << "Decoding: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << opcode << 
        std::endl;

    if (opcode == kClearScreen) {
        clearScreen();
        return;
    }

    if (opcode == kReturn) {
        returnFromSubroutine();
    }

    switch (opcode & 0xF000) {
        case kJump:
            jump(opcode);
            break;
        case kCall:
            callSubroutine(opcode);
            break;
        case kSkipEqual:
            skipIfEqual(opcode);
        case kSetVxReg:
            setVxRegister(opcode);
            break;
        case kAddValueToVxReg:
            addValueToVxRegister(opcode);
            break;
        case kSetIndexRegI:
            setIndexRegister(opcode);
            break;
        case kDisplayDraw:
            displayDraw(opcode);
            break;
        default:
            std::cerr << "Unknown opcode: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase 
                << opcode << std::endl;
            break;
    }
}

void Chip8::jump(uint16_t opcode) {
    reg_.PC = opcode & 0x0FFF;
    std::cout << "Jump to 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << reg_.PC << 
        std::endl;
}

void Chip8::callSubroutine(uint16_t opcode) {
    ++reg_.SP;
    reg_.stack.push(reg_.PC);
    reg_.PC = opcode & 0x0FFF;
}

void Chip8::skipIfEqual(uint16_t opcode) {
    uint8_t v_reg = static_cast<uint8_t>(((opcode & 0x0F00) >> 8));
    if (reg_.V[v_reg] == static_cast<uint8_t>(opcode & 0x00FF)) {
        reg_.PC += 2;
    }
}

void Chip8::setVxRegister(uint16_t opcode) {
    uint8_t v_reg = static_cast<uint8_t>(((opcode & 0x0F00) >> 8));
    reg_.V[v_reg] = static_cast<uint8_t>(opcode & 0x00FF);
    std::cout << "Set V[" << std::setw(1) << std::hex << std::uppercase << int(v_reg) << "] = " << 
        std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(reg_.V[v_reg]) << std::endl;
}


void Chip8::addValueToVxRegister(uint16_t opcode) {
    uint8_t v_reg = static_cast<uint8_t>(((opcode &0x0F00) >> 8));
    reg_.V[v_reg] += static_cast<uint8_t>(opcode & 0x00FF);
    std::cout << "Add " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << 
        int(static_cast<uint8_t>(opcode & 0x00FF)) << " to V[" << std::setw(1) << std::hex << std::uppercase 
        << int(v_reg) << "]\n";
}

void Chip8::setIndexRegister(uint16_t opcode) {
    reg_.I = (opcode & 0x0FFF);
    std::cout << "I = 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << reg_.I << 
        std::endl;
}

void Chip8::displayDraw(uint16_t opcode) {
    uint8_t vx_pos = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
    uint8_t display_x_pos = reg_.V[vx_pos] % kDisplayWidth;
    uint8_t vy_pos = static_cast<uint8_t>((opcode & 0x00F0) >> 4);
    uint8_t display_y_pos = reg_.V[vy_pos] % kDisplayHeight;
    uint8_t bytes_to_read = static_cast<uint8_t>(opcode & 0x000F);
    auto start_x_pos = display_x_pos;
    reg_.V[0xF] = 0x00;

    // TODO: Implement the correct logic
    // We update our screen buffer before actually drawing + rendering
    for(uint32_t i = 0; i < bytes_to_read; ++i) {
        if (display_y_pos >= kDisplayHeight) {
            break;
        }

        display_x_pos = start_x_pos;
        const auto &display_data = memory_[reg_.I + i];
        for (int8_t j = 7; j >= 0; --j) {
            if (display_x_pos >= kDisplayWidth) {
                break;
            }

            const uint8_t &sprite_pixel = (display_data >> j) & 0x01;
            // (Y position * display width) walks the pointer on the vertical line
            auto *screen_buf_pixel = &screen_buffer_[display_x_pos + display_y_pos * kDisplayWidth];
            if (sprite_pixel && *screen_buf_pixel) {
                reg_.V[0xF] = 0x01;
            }

            *screen_buf_pixel ^= sprite_pixel;
            ++display_x_pos;
        }
          ++display_y_pos;
    }

    display_->render(screen_buffer_);
}

void Chip8::clearScreen(void) {
    std::cout << "clearScreen called\n";
    std::memset(screen_buffer_, 0, sizeof(screen_buffer_));
    display_->clear();
}

void Chip8::returnFromSubroutine(void) {
    reg_.PC = reg_.stack.top();
    reg_.stack.pop();
    --reg_.SP;
}
