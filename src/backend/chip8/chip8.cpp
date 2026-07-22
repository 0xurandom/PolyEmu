#include "chip8.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>

Opcode Chip8::getOpcode() {
    Opcode opcode = {.code =
                         static_cast<uint16_t>((ram[pc] << 8) | ram[pc + 1])};
    // TODO: do not increment pc if opcode is jump
    pc += 2;

    return opcode;
}

bool Chip8::loadROM(const std::string &filepath) {
    const unsigned int startAddr = 0x200;
    const std::streamsize maxSize = sizeof(ram) - startAddr;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open ROM" << std::endl;
        return false;
    }

    if (file.tellg() > maxSize) {
        std::cerr << "Error: ROM size is larger than can be allocated in ram"
                  << std::endl;
        return false;
    }

    if (!file.read(reinterpret_cast<char *>(&ram[startAddr]), file.tellg()))
        return false;
    else
        return true;
}

void Chip8::handleOpcode(Opcode opcode) {
    uint16_t firstNibble = (opcode.code & 0xF000) >> 12;

    switch (firstNibble) {
        case 0x0: {
            switch (opcode.NN) {
                case 0xE0:
                    clearScreen();
                    break;
                case 0xEE:  // TODO subroutine

                default: {
                    std::cerr << "Error: Unknown opcode starting with 0x0"
                              << std::endl;
                    break;
                }
            }

            break;
        }

        // 1NNN
        case 0x1: {
            jump(opcode);
            break;
        }

        // 2NNN
        case 0x2: {
            // TODO
            break;
        }

        // 3XNN
        case 0x3: {
            if (V[opcode.X] == opcode.NN) skipInstruction();
            break;
        }

        // 4XNN
        case 0x4: {
            if (V[opcode.X] != opcode.NN) skipInstruction();
            break;
        }

        // 5XY0
        case 0x5: {
            if (V[opcode.X] == V[opcode.Y]) skipInstruction();
            break;
        }

        // 9XY0
        case 0x9: {
            if (V[opcode.X] != V[opcode.Y]) skipInstruction();
            break;
        }

        // 6XNN
        case 0x6: {
            setVarRegister(opcode);
            break;
        }

        // 7XNN
        case 0x7: {
            addValToRegister(opcode);
            break;
        }

        case 0x8: {
            switch (opcode.N) {
                case 0x0: {
                    // 8XY0
                    setVXtoVY(opcode);
                    break;
                }

                case 0x1: {
                    // 8XY1
                    binaryOr(opcode);
                    break;
                }
            }

            break;
        }
    }
}

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

void Chip8::setVXtoVY(Opcode opcode) { V[opcode.X] = V[opcode.Y]; }

void Chip8::setIndex(Opcode opcode) { I = opcode.NNN; }

void Chip8::binaryOr(Opcode opcode) { V[opcode.X] = V[opcode.X] | V[opcode.Y]; }

void Chip8::skipInstruction() { pc += 2; }

void Chip8::jump(Opcode opcode) { pc = opcode.NNN; }
