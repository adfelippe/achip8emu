#include "chip8.hpp"

#include <cstring>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <thread>

Chip8::Chip8(size_t memory_start_offset) : memory_start_offset_(memory_start_offset) {
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
    memcpy(&memory_[kSpritesMemLocation], hex_sprites, sizeof(hex_sprites));
    memset(&reg_, 0, sizeof(Register));
}

Chip8::Chip8() : Chip8(kMemoryStartOffsetDefault) {}

void Chip8::load(const std::string &path) {
    std::cout << "Loading " << path << "\n";

    std::ifstream f;
    f.open(path.c_str(), (std::ios::in | std::ios::binary));

    if (!f.good()) {
        throw LoadFileException();
    }

    std::filesystem::path file_path = path;
    auto file_size = std::filesystem::file_size(file_path);
    program_data_.reserve(file_size);
    f.read(reinterpret_cast<char *>(program_data_.data() + memory_start_offset_), file_size);
    reg_.PC = memory_start_offset_;
    std::cout << file_size << " bytes loaded successfully\n";
    f.close();
}

void Chip8::run(void) {
    while (1) {
        auto instruction = fetchInstruction();
        decodeInstruction(instruction);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
    uint16_t instruction = ((static_cast<uint16_t>(program_data_[reg_.PC]) << 8) & 0xFF00) + program_data_[reg_.PC + 1];
    reg_.PC += 2;
    return instruction;
}

void Chip8::decodeInstruction(uint16_t instruction) {
    std::cout << "Decoding: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << instruction << 
        std::endl;

    if (instruction == kClearScreen) {
        clearScreen();
        return;
    }

    switch (instruction & 0xF000) {
        case kJump:
            jump(instruction);
            break;
        case kSetVxReg:
            setVxRegister(instruction);
            break;
        case kAddValueToVxReg:
            addValueToVxRegister(instruction);
            break;
        case kSetIndexRegI:
            setIndexRegister(instruction);
            break;
        case kDisplayDraw:
            displayDraw(instruction);
            break;
        default:
            std::cerr << "Unknown instruction: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase 
                << instruction << std::endl;
            break;
    }
}

void Chip8::jump(uint16_t instruction) {
    reg_.PC = instruction & 0x0FFF;
    std::cout << "Jump to 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << reg_.PC << 
        std::endl;
}

void Chip8::setVxRegister(uint16_t instruction) {
    uint8_t v_reg = static_cast<uint8_t>(((instruction & 0x0F00) >> 8));
    reg_.V[v_reg] = static_cast<uint8_t>(instruction & 0x00FF);
    std::cout << "Set V[" << std::setw(1) << std::hex << std::uppercase << int(v_reg) << "] = " << 
        std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(reg_.V[v_reg]) << std::endl;
}


void Chip8::addValueToVxRegister(uint16_t instruction) {
    uint8_t v_reg = static_cast<uint8_t>(((instruction &0x0F00) >> 8));
    reg_.V[v_reg] += static_cast<uint8_t>(instruction & 0x00FF);
    std::cout << "Add " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << 
        int(static_cast<uint8_t>(instruction & 0x00FF)) << " to V[" << std::setw(1) << std::hex << std::uppercase 
        << int(v_reg) << "]\n";
}

void Chip8::setIndexRegister(uint16_t instruction) {
    reg_.I = (instruction & 0x0FFF);
    std::cout << "I = 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << reg_.I << 
        std::endl;
}

void Chip8::displayDraw(uint16_t instruction) {
    uint8_t bytes_to_read = static_cast<uint8_t>(instruction & 0x000F);
    std::vector<uint8_t>::const_iterator data_start = program_data_.begin() + reg_.I;
    std::vector<uint8_t>::const_iterator data_end = program_data_.begin() + reg_.I + bytes_to_read;
    // Adds 1 because the last position is not included [start, end)
    std::vector<uint8_t> loaded_data(data_start, data_end + 1);

    for (size_t i = 0; i < loaded_data.size(); ++i) {
        std::cout << " " << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << int(loaded_data[i]);

        if (i != 0 && i % 63 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

void Chip8::clearScreen(void) {
    std::cout << "clearScreen called\n";
}
