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

void Pico8::handleOpcode(Opcode opcode) { switch () }

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
