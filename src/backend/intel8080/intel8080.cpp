#include "intel8080.hpp"

#include <raylib.h>

#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>

bool i8080::loadROM(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open i8080 file" << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    filebuffer.reserve(size);

    if (!file.read(reinterpret_cast<char *>(filebuffer.data()), size)) {
        std::cerr << "Error: Could not read i8080 file" << std::endl;
        return false;
    }

    return true;
}

void i8080::emulate8080() {
    unsigned char *opcode = &state.memory[state.pc];

    switch (*opcode) {
        // NOP
        case 0x00:
            break;

        // LXI B, word
        case 0x01: {
            state.c = opcode[1];
            state.b = opcode[2];
            state.pc += 2;
            break;
        }

        // MOV B, C
        case 0x41: {
            state.b = state.c;
            break;
        }

        // MOV B, D
        case 0x42: {
            state.b = state.d;
            break;
        }

        // MOV B, E
        case 0x43: {
            state.b = state.e;
            break;
        }

        // ADD B
        case 0x80: {
            uint16_t ans =
                static_cast<uint16_t>(state.a) + static_cast<uint16_t>(state.b);

            if ((ans & 0xff) == 0)
                state.cc.z = 1;
            else
                state.cc.z = 0;

            if (ans & 0x80)
                state.cc.s = 1;
            else
                state.cc.s = 0;

            if (ans > 0xff)
                state.cc.cy = 1;
            else
                state.cc.cy = 0;

            // TODO
            // state.cc.p = parity()

            state.a = ans & 0xff;

            break;
        }

        // ADD C
        case 0x81: {
            uint16_t ans =
                static_cast<uint16_t>(state.a) + static_cast<uint16_t>(state.c);

            state.cc.z = ((ans & 0xff) == 0) ? 1 : 0;
            state.cc.s = (ans & 0x80) ? 1 : 0;
            state.cc.cy = (ans > 0xff) ? 1 : 0;
            // state.cc.p
            state.a = ans & 0xff;
            break;
        }
    }

    state.pc++;
}

void i8080::unimplementedInstruction() {
    std::cerr << "Error: Unimplemented instruction" << std::endl;

    state.pc--;
    exit(1);
}
