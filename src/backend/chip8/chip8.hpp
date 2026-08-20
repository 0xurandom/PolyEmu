#pragma once

#include <cstdint>
#include <iostream>
#include <string>

#include "../backend.hpp"

struct Opcode {
    uint16_t code;

    uint8_t X;
    uint8_t Y;
    uint8_t N;
    uint8_t NN;
    uint16_t NNN;

    explicit Opcode(uint16_t inputCode)
        : code(inputCode),
          X((inputCode & 0x0F00) >> 8),
          Y((inputCode & 0x00F0) >> 4),
          N(inputCode & 0x000F),
          NN(inputCode & 0x00FF),
          NNN(inputCode & 0x0FFF) {}
};

const char chip8Buttons[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

enum class Chip8Key {
    Zero = 0x0,
    One = 0x1,
    Two = 0x2,
    Three = 0x3,
    Four = 0x4,
    Five = 0x5,
    Six = 0x6,
    Seven = 0x7,
    Eight = 0x8,
    Nine = 0x9,

    A = 0xA,
    B = 0xB,
    C = 0xC,
    D = 0xD,
    E = 0xE,
    F = 0xF,
};

class Chip8 : public EmuBackend {
   public:
    Chip8() { reset(); }

    bool loadROM(const std::string& filepath) override;
    void reset() override;
    // void runEmuFrame() override;

    Opcode getOpcode();
    void handleOpcode(Opcode opcode);

    bool saveState(const std::string& path);
    bool loadState(const std::string& path);

    uint8_t getSoundTimer() { return sound_timer; }

    void runTimers();

    void loadFonts();

    uint8_t* getDisplay() { return display; }

    static constexpr int displayWidth = 64;
    static constexpr int displayHeight = 32;
    int getDisplayWidth() const override { return displayWidth; }
    int getDisplayHeight() const override { return displayHeight; }

    void setKeyState(int key, bool state) {
        if (key < 0 || key > 15) {
            std::cerr << "Error: Unknown key passed to setKeystate\n";
            return;
        }

        keys[key] = state;
    }

   private:
    // display
    uint8_t display[displayWidth * displayHeight] = {0};

    // 4kb ram
    uint8_t ram[4096] = {0};

    // keys
    bool keys[16] = {false};

    // index register
    uint16_t I = 0;

    // stack for funcs/subroutines
    uint16_t stack[16] = {0};

    uint8_t sp = 0;

    // var register
    uint8_t V[16] = {0};

    // fonts
    static constexpr uint8_t fonts[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
    };

    uint16_t pc = 0x200;

    // delay timer
    uint8_t delay_timer = 0;

    // sound timer
    uint8_t sound_timer = 0;

    void draw(Opcode opcode);
    void clearScreen();

    void callSubroutine(Opcode opcode);
    void returnFromSubroutine();

    void setVarRegister(Opcode opcode);
    void addValToRegister(Opcode opcode);
    void addValToRegisterOverflow(Opcode opcode);
    void setVXtoVY(Opcode opcode);
    void binaryOr(Opcode opcode);
    void binaryAnd(Opcode opcode);
    void binaryXor(Opcode opcode);
    void subtractYfromX(Opcode opcode, bool* underflow);
    void subtractXfromY(Opcode opcode, bool* underflow);
    void shiftRight(Opcode opcode, uint8_t* shiftedBit);
    void shiftLeft(Opcode opcode, uint8_t* shiftedBit);
    void skipInstruction();
    void setIndex(Opcode opcode);
    void jump(Opcode opcode);
    void jumpWithOffset(Opcode opcode);
    void randomNum(Opcode opcode);
    void skipIfKey(Opcode opcode);
    void skipIfNotKey(Opcode opcode);
    void setVXToDelay(Opcode opcode);
    void setDelayToVX(Opcode opcode);
    void setSoundToVX(Opcode opcode);
    void addToIndex(Opcode opcode, bool* overflow);
    void getKey(Opcode opcode);
    void setFont(Opcode opcode);
    void binaryDecimalConversion(Opcode opcode);
    void storeMem(Opcode opcode);
    void loadMem(Opcode opcode);

    bool isChip8KeyPressed();
};
