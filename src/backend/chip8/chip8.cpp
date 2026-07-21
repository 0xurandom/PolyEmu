#include "chip8.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

Opcode Chip8::getOpcode() {
    Opcode opcode = {.code =
                         static_cast<uint16_t>((ram[pc] << 8) | ram[pc + 1])};
    // TODO: do not increment pc if opcode is jump
    pc += 2;

    return opcode;
}

// void Pico8::handleOpcode(Opcode opcode) { switch () };

void Chip8::draw(Opcode opcode) {
    uint8_t VX = V[opcode.X];
    uint8_t VY = V[opcode.Y];

    uint8_t startX = VX % displayWidth;
    uint8_t startY = VY % displayHeight;

    V[15] = 0;

    for (size_t row = 0; row < opcode.N; row++) {
        uint8_t Y = startY + row;
        if (Y >= displayHeight) break;

        uint8_t spriteByte = ram[I + row];

        for (size_t col = 0; col < 8; col++) {
            uint8_t X = startX + col;
            if (X >= displayWidth) break;

            uint8_t spriteBit = (spriteByte >> (7 - col)) & 0x1;
            uint8_t screenBit = display[(Y * displayWidth) + X];

            if ((spriteBit == 1) && (screenBit == 1)) {
                display[(Y * displayWidth) + X] = 0;
                V[15] = 1;

            } else if ((spriteBit == 1) && (screenBit == 0)) {
                screenBit = 1;
            }
        }
    }
}

void Chip8::clearScreen() { std::memset(display, 0, sizeof(display)); }

void Chip8::setVarRegister(Opcode opcode) {
    if (opcode.X < 0 || opcode.X > 15) {
        std::cerr << "Error: setVarRegister received invalid register num"
                  << std::endl;
    }

    V[opcode.X] = opcode.NN;
}

void Chip8::addValToRegister(Opcode opcode) {
    if (opcode.X < 0 || opcode.X > 15) {
        std::cerr << "Error: addValToRegister received invalid register num"
                  << std::endl;
    }

    V[opcode.X] += opcode.NN;
}

void Chip8::setIndex(Opcode opcode) { I = opcode.NNN; }

void Chip8::jump(Opcode opcode) { pc = opcode.NNN; }
