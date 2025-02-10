#include "Chip8.hpp"

#include <cstring>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <thread>
#include <random>
#include <SDL2/SDL.h>

using namespace std::chrono_literals;

static const std::map<char, uint8_t> keyboardToChip8Map = {
    {'1', 0x01},
    {'2', 0x02},
    {'3', 0x03},
    {'4', 0x0C},
    {'Q', 0x04},
    {'W', 0x05},
    {'E', 0x06},
    {'R', 0x0D},
    {'A', 0x07},
    {'S', 0x08},
    {'D', 0x09},
    {'F', 0x0E},
    {'Z', 0x0A},
    {'X', 0x00},
    {'C', 0x0B},
    {'V', 0x0F}
};

Chip8::Chip8(size_t memory_start_offset, const std::shared_ptr<IDisplay> &display, const std::shared_ptr<IKeyboard> &keyboard) : 
    memory_start_offset_(memory_start_offset),
    display_(display),
    keyboard_(keyboard) {
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

Chip8::Chip8(const std::shared_ptr<IDisplay> &display, const std::shared_ptr<IKeyboard> &keyboard) : 
    Chip8(kMemoryStartOffsetDefault, display, keyboard) {}

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

    if (file_size > kMemorySize - kMemoryStartOffsetDefault) {
        throw Chip8Exception("File too large: " + std::to_string(file_size) +  " bytes");
    }

    reg_.PC = memory_start_offset_;
    f.read((char*)&memory_[memory_start_offset_], file_size);
    std::cout << file_size << " bytes loaded successfully\n";
    f.close();
}

void Chip8::run(void) {
    auto start_time = std::chrono::steady_clock::now();

    while (!keyboard_->quitClicked()) {
        auto start_refresh_delay = std::chrono::steady_clock::now();
        auto opcode = fetchInstruction();
        decodeInstruction(opcode);

        // 60 Hz refresh rate
        if (std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start_time).count() > kCpuPeriodUs) {
            start_time = std::chrono::steady_clock::now();
            display_->render(screen_buffer_);
            runDelayTimer();
            runSoundTimer();
        }

        // For now, we'll stick to 500 Hz CPU frequency.
        // TODO: Tweak each instructions to take a more realistic time based on the original HW:
        // https://jackson-s.me/2019/07/13/Chip-8-Instruction-Scheduling-and-Frequency.html
        std::this_thread::sleep_until(start_refresh_delay + 2ms);
    }
}

void Chip8::runDelayTimer(void) {
    if (reg_.DT) {
        --reg_.DT;
    }
}

void Chip8::runSoundTimer(void) {
    if (reg_.ST) {
        buzzerOn();
        --reg_.ST;
    } else {
        buzzerOff();
    }
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
    if (opcode == kClearScreen) {
        clearScreen();
        return;
    }

    if (opcode == kReturn) {
        returnFromSubroutine();
        return;
    }

    uint8_t x = static_cast<uint8_t>((opcode >> 8) & 0x000F);
    uint8_t y = static_cast<uint8_t>((opcode >> 4) & 0x000F);
    uint8_t kk = static_cast<uint8_t>(opcode & 0x00FF);
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = static_cast<uint8_t>(opcode & 0x000F);

    /*std::cout << "OPCODE [" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << opcode << "]";
    std::cout << ", x = " << std::setfill('0') << std::setw(1) << std::hex << std::uppercase << int(x);
    std::cout << ", y = " << std::setfill('0') << std::setw(1) << std::hex << std::uppercase << int(y);
    std::cout << ", kk = " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(kk);
    std::cout << ", nnn = " << std::setfill('0') << std::setw(3) << std::hex << std::uppercase << int(nnn);
    std::cout << ", n = " << std::setfill('0') << std::setw(1) << std::hex << std::uppercase << int(n) << "\n";*/

    switch ((opcode >> 12) & 0x000F) {
        case kJump:
            jump(nnn);
            break;
        case kCall:
            callSubroutine(nnn);
            break;
        case kSkipEqual:
            skipIfEqual(x, kk);
            break;
        case kSetVxReg:
            setVxRegister(x, kk);
            break;
        case kAddValueToVxReg:
            addValueToVxRegister(x, kk);
            break;
        case kVRegOperation:
            runVRegOperation(x, y, n);
            break;
        case kSkipNotEqual:
            skipNext(x, y);
            break;
        case kJumpToAddrPlusV0:
            jumpToAddrPlusV0(nnn);
            break;
        case kSetRandom:
            setRandomByteToVx(x, kk);
            break;
        case kSetIndexRegI:
            setIndexRegister(nnn);
            break;
        case kDisplayDraw:
            displayDraw(x, y, n);
            break;
        case kSkipNetIfKey:
            skipNetIfKey(x, y, n);
            break;
        case kMisc:
            decodeMisc(x, y, n);
            break;
        default:
            std::cerr << "UNKNOWN OPCODE = " << std::setfill('0') << std::setw(4) << std::hex << std::uppercase 
                << opcode << "\n";
            break;
    }
}

void Chip8::jump(uint16_t address) {
    reg_.PC = address;
}

void Chip8::callSubroutine(uint16_t address) {
    ++reg_.SP;
    reg_.stack.push(reg_.PC);
    reg_.PC = address;
}

void Chip8::skipIfEqual(uint8_t v_reg, uint8_t value) {
    if (reg_.V[v_reg] == value) {
        reg_.PC += 2;
    }
}

void Chip8::setVxRegister(uint8_t v_reg, uint8_t value) {
    reg_.V[v_reg] = value;
}


void Chip8::addValueToVxRegister(uint8_t v_reg, uint8_t value) {
    reg_.V[v_reg] += value;
}

void Chip8::clearScreen(void) {
    std::memset(screen_buffer_, 0, sizeof(screen_buffer_));
    display_->clear();
}

void Chip8::returnFromSubroutine(void) {
    reg_.PC = reg_.stack.top();
    reg_.stack.pop();
    --reg_.SP;
}

void Chip8::runVRegOperation(uint8_t x, uint8_t y, uint8_t n) {
    switch (n) {
        case 0x00: {
            reg_.V[x] = reg_.V[y];
            break;
        }
        case 0x01: {
            reg_.V[x] |= reg_.V[y];
            break;
        }
        case 0x02: {
            reg_.V[x] &= reg_.V[y];
            break;
        }
        case 0x03: {
            reg_.V[x] ^= reg_.V[y];
            break;
        }
        case 0x04: {
            uint16_t result = static_cast<uint16_t>(reg_.V[x]) + static_cast<uint16_t>(reg_.V[y]);
            reg_.V[x] =  static_cast<uint8_t>(result & 0x00FF);
            reg_.V[0xF] = static_cast<uint8_t>((result >> 8) & 0x01);
            break;
        }
        case 0x05: {
            reg_.V[0xF] = (reg_.V[x] > reg_.V[y]);
            reg_.V[x] -= reg_.V[y];
            break;
        }
        case 0x06: {
            reg_.V[0xF] = reg_.V[x] & 0x01;
            reg_.V[x] = reg_.V[x] >> 1;
            break;
        }
        case 0x07: {
            reg_.V[0xF] = (reg_.V[y] > reg_.V[x]);
            reg_.V[x] = reg_.V[y] - reg_.V[x];
            break;
        }
        case 0x0E: {
            reg_.V[0xF] = reg_.V[x] & 0x80;
            reg_.V[x] = reg_.V[x] << 1;
            break;
        }
        default:
            break;
    }
}

void Chip8::skipNext(uint8_t x, uint8_t y) {
    if (reg_.V[x] != reg_.V[y]) {
        reg_.PC += 2;
    }
}


void Chip8::setIndexRegister(uint16_t value) {
    reg_.I = value;
}

void Chip8::jumpToAddrPlusV0(uint16_t address) {
    reg_.PC = address + reg_.V[0x00];
}

void Chip8::setRandomByteToVx(uint8_t v_reg, uint8_t value) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist_0_255(0, 255);
    uint8_t rnd_number = dist_0_255(rng);

    reg_.V[v_reg] = rnd_number & value;
}

void Chip8::displayDraw(uint8_t x, uint8_t y, uint8_t n) {
    uint8_t display_x_pos = reg_.V[x] % kDisplayWidth;
    uint8_t display_y_pos = reg_.V[y] % kDisplayHeight;
    auto start_x_pos = display_x_pos;
    reg_.V[0xF] = 0x00;

    // We update our screen buffer before actually rendering
    for(uint32_t i = 0; i < n; ++i) {
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
}

void Chip8::skipNetIfKey(uint8_t x, uint8_t y, uint8_t n) {
    // SKP Vx
    if (y == 0x09 && n == 0x0E && keyIsPressed(x)) {
        reg_.PC += 2;
    // SKNP Vx
    } else if (y == 0x0A && n == 0x01 && !keyIsPressed(x)) {
        reg_.PC += 2;
    }
}

void Chip8::decodeMisc(uint8_t x, uint8_t y, uint8_t n) {
    uint16_t subcode = (static_cast<uint16_t>(y) << 8) + static_cast<uint16_t>(n);

    switch (subcode) {
        case kMiscDelayTimerValue:
            reg_.V[x] = reg_.DT;
            break;
        case kMiscWaitForKey: {
            uint8_t keyValue = 0xFF;
            auto isPressed = getKey(&keyValue);
            if (!isPressed) {
                reg_.PC -= 2;
            } else {
                reg_.V[x] = keyValue;
            }
        }
            break;
        case kMiscSetDelayTimer:
            reg_.DT = reg_.V[x];
            break;
        case kMiscSetSoundTimer:
            reg_.ST = reg_.V[x];
            break;
        case kMiscAddToIndex:
            reg_.I += reg_.V[x];
            break;
        case kMiscFontChar:
            reg_.I = memory_[reg_.V[x] * 5];
            break;
        case kMiscStoreBcd:
            memory_[reg_.I] = reg_.V[x] / 100;
            memory_[reg_.I + 1] = (reg_.V[x] / 10) % 10;
            memory_[reg_.I + 2] = reg_.V[x] % 10;
            break;
        case kMiscStoreMemory:
            for (uint8_t i = 0; i <= x; ++i) {
                memory_[reg_.I + i] = reg_.V[i];
            }
            break;
        case kMiscLoadMemory:
            for (uint8_t i = 0; i <= x; ++i) {
                reg_.V[i] = memory_[reg_.I + i];
            }
            break;
        default:
            break;
    }
}

bool Chip8::keyIsPressed(uint8_t x) {
    auto key = keyboard_->getKey();

    if (key.state == IKeyboard::Key::State::kReleased) {
        return false;
    }

    if (isKeyChip8Valid(key.value) && key.value == reg_.V[x]) {
        return true;
    }

    return false;
}

bool Chip8::isKeyChip8Valid(char value) {
    uint8_t chip8Key = convertKeyboardToChip8(value);
    return (chip8Key >= 0x00 && chip8Key <= 0x0F);
}

uint8_t Chip8::convertKeyboardToChip8(char value) {
    auto upperChar = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));

    if (keyboardToChip8Map.count(upperChar) > 0) {
        return keyboardToChip8Map.at(upperChar);
    }

    return 0xFF;
}

bool Chip8::getKey(uint8_t *keyValue) {
    auto key = keyboard_->getKey();

    if (key.state == IKeyboard::Key::State::kReleased || !isKeyChip8Valid(key.value)) {
        return false;
    }

    *keyValue = convertKeyboardToChip8(key.value);
    return true;
}

