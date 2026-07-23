#pragma once

#include <cstdint>
#include <vector>

#include "../backend.hpp"

struct Opcode {
    uint16_t code;

    uint8_t X = (code & 0xF000) >> 8;
    uint8_t Y = (code & 0x0F00) >> 8;
    uint8_t N = (code & 0x00F0);
    uint8_t NN = (code & 0x00FF);
    uint16_t NNN = (code & 0x0FFF);
};

class Chip8 : public EmuBackend {
   public:
    bool loadROM(const std::string &filepath) override;

    static constexpr int displayWidth = 64;
    static constexpr int displayHeight = 32;
    int getDisplayWidth() const override { return displayWidth; }
    int getDisplayHeight() const override { return displayHeight; }

   private:
    // display
    uint8_t display[displayWidth * displayHeight] = {0};

    // 4kb ram
    uint8_t ram[4096] = {0};

    // index register
    uint16_t I = 0;

    // stack for funcs/subroutines
    uint16_t stack[16] = {0};

    // var register
    uint8_t V[16] = {0};

    uint16_t pc = 0;

    // delay timer
    uint8_t delay_timer = 0;

    // sound timer
    uint8_t sound_timer = 0;

    Opcode getOpcode();

    void handleOpcode(Opcode opcode);

    void clearScreen();
    void draw(Opcode opcode);
    void setVarRegister(Opcode opcode);
    void addValToRegister(Opcode opcode, bool *overflow);
    void setVXtoVY(Opcode opcode);
    void binaryOr(Opcode opcode);
    void binaryAnd(Opcode opcode);
    void binaryXor(Opcode opcode);
    void subtractYfromX(Opcode opcode, bool *underflow);
    void subtractXfromY(Opcode opcode, bool *underflow);
    void shiftRight(Opcode opcode, uint8_t *shiftedBit);
    void shiftLeft(Opcode opcode, uint8_t *shiftedBit);
    void skipInstruction();
    void setIndex(Opcode opcode);
    void jump(Opcode opcode);
    void jumpWithOffset(Opcode opcode);
    void randomNum(Opcode opcode);
};
