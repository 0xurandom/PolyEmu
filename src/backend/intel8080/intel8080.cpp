#include "intel8080.hpp"

#include <raylib.h>

#include <bit>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>

bool i8080::loadROM(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open i8080 file" << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    filebuffer.reserve(size);

    if (!file.read(reinterpret_cast<char*>(filebuffer.data()), size)) {
        std::cerr << "Error: Could not read i8080 file" << std::endl;
        return false;
    }

    return true;
}

void i8080::emulate8080() {
    unsigned char* opcode = &state.memory[state.pc];

    switch (*opcode) {
        // NOP
        case 0x00: break;

        // LXI B, word
        case 0x01: {
            state.c = opcode[1];
            state.b = opcode[2];
            state.pc += 2;
            break;
        }

        case 0x2f: state.a = ~state.a; break;  // CMA

        case 0x40: state.b = state.b; break;            // MOV B, B
        case 0x41: state.b = state.c; break;            // MOV B, C
        case 0x42: state.b = state.d; break;            // MOV B, D
        case 0x43: state.b = state.e; break;            // MOV B, E
        case 0x44: state.b = state.h; break;            // MOV B, H
        case 0x45: state.b = state.l; break;            // MOV B, L
        case 0x46: state.b = *state.getHLPtr(); break;  // MOV B, M
        case 0x47: state.b = state.a; break;            // MOV B, A
        case 0x48: state.c = state.b; break;            // MOV C, B
        case 0x49: state.c = state.c; break;            // MOV C, C
        case 0x4a: state.c = state.d; break;            // MOV C, D
        case 0x4b: state.c = state.e; break;            // MOV C, E
        case 0x4c: state.c = state.h; break;            // MOV C, H
        case 0x4d: state.c = state.l; break;            // MOV C, L
        case 0x4e: state.c = *state.getHLPtr(); break;  // MOV C, M
        case 0x4f: state.c = state.a; break;            // MOV C, A
        case 0x50: state.d = state.b; break;            // MOV D, B
        case 0x51: state.d = state.c; break;            // MOV D, C
        case 0x52: state.d = state.d; break;            // MOV D, D
        case 0x53: state.d = state.e; break;            // MOV D, E
        case 0x54: state.d = state.h; break;            // MOV D, H
        case 0x55: state.d = state.l; break;            // MOV D, L
        case 0x56: state.d = *state.getHLPtr(); break;  // MOV D, M
        case 0x57: state.d = state.a; break;            // MOV D, A
        case 0x58: state.e = state.b; break;            // MOV E, B
        case 0x59: state.e = state.c; break;            // MOV E, C
        case 0x5a: state.e = state.d; break;            // MOV E, D
        case 0x5b: state.e = state.e; break;            // MOV E, E
        case 0x5c: state.e = state.h; break;            // MOV E, H
        case 0x5d: state.e = state.l; break;            // MOV E, L
        case 0x5e: state.e = *state.getHLPtr(); break;  // MOV E, M
        case 0x5f: state.e = state.a; break;            // MOV E, A
        case 0x60: state.h = state.b; break;            // MOV H, B
        case 0x61: state.h = state.c; break;            // MOV H, C
        case 0x62: state.h = state.d; break;            // MOV H, D
        case 0x63: state.h = state.e; break;            // MOV H, E
        case 0x64: state.h = state.h; break;            // MOV H, H
        case 0x65: state.h = state.l; break;            // MOV H, L
        case 0x66: state.h = *state.getHLPtr(); break;  // MOV H, M
        case 0x67: state.h = state.a; break;            // MOV H, A
        case 0x68: state.l = state.b; break;            // MOV L, B
        case 0x69: state.l = state.c; break;            // MOV L, C
        case 0x6a: state.l = state.d; break;            // MOV L, D
        case 0x6b: state.l = state.e; break;            // MOV L, E
        case 0x6c: state.l = state.h; break;            // MOV L, H
        case 0x6d: state.l = state.l; break;            // MOV L, L
        case 0x6e: state.l = *state.getHLPtr(); break;  // MOV L, M
        case 0x6f: state.l = state.a; break;            // MOV L, A
        case 0x70: *state.getHLPtr() = state.b;         // MOV M, B
        case 0x71: *state.getHLPtr() = state.c;         // MOV M, C
        case 0x72: *state.getHLPtr() = state.d;         // MOV M, D
        case 0x73: *state.getHLPtr() = state.e;         // MOV M, E
        case 0x74: *state.getHLPtr() = state.h;         // MOV M, H
        case 0x75: *state.getHLPtr() = state.l;         // MOV M, L
        case 0x76: break;                               // TODO HLT
        case 0x77: *state.getHLPtr() = state.a;         // MOV M, A
        case 0x78: state.a = state.b; break;            // MOV A, B
        case 0x79: state.a = state.c; break;            // MOV A, C
        case 0x7a: state.a = state.d; break;            // MOV A, D
        case 0x7b: state.a = state.e; break;            // MOV A, E
        case 0x7c: state.a = state.h; break;            // MOV A, H
        case 0x7d: state.a = state.h; break;            // MOV A, H
        case 0x7e: state.a = *state.getHLPtr(); break;  // MOV A, M
        case 0x7f: state.a = state.a; break;            // MOV A, A

        case 0x80: iadd(state.b); break;            // ADD B
        case 0x81: iadd(state.c); break;            // ADD C
        case 0x82: iadd(state.d); break;            // ADD D
        case 0x83: iadd(state.e); break;            // ADD E
        case 0x84: iadd(state.h); break;            // ADD H
        case 0x85: iadd(state.l); break;            // ADD L
        case 0x86: iadd(*state.getHLPtr()); break;  // ADD M
        case 0x87: iadd(state.a); break;            // ADD A

        case 0x88: iadc(state.b); break;            // ADC B
        case 0x89: iadc(state.c); break;            // ADC C
        case 0x8a: iadc(state.d); break;            // ADC D
        case 0x8b: iadc(state.e); break;            // ADC E
        case 0x8c: iadc(state.h); break;            // ADC H
        case 0x8d: iadc(state.l); break;            // ADC L
        case 0x8e: iadc(*state.getHLPtr()); break;  // ADC M
        case 0x8f: iadc(state.a); break;            // ADC A

        case 0x90: isub(state.b); break;            // SUB B
        case 0x91: isub(state.c); break;            // SUB C
        case 0x92: isub(state.d); break;            // SUB D
        case 0x93: isub(state.e); break;            // SUB E
        case 0x94: isub(state.h); break;            // SUB H
        case 0x95: isub(state.l); break;            // SUB L
        case 0x96: isub(*state.getHLPtr()); break;  // SUB M
        case 0x97: isub(state.a); break;            // SUB A

        case 0x98: isbb(state.b); break;            // SBB B
        case 0x99: isbb(state.c); break;            // SBB C
        case 0x9a: isbb(state.d); break;            // SBB D
        case 0x9b: isbb(state.e); break;            // SBB E
        case 0x9c: isbb(state.h); break;            // SBB H
        case 0x9d: isbb(state.l); break;            // SBB L
        case 0x9e: isbb(*state.getHLPtr()); break;  // SBB M
        case 0x9f: isbb(state.a); break;            // SBB A

        case 0xa0: iana(state.b); break;            // ANA B
        case 0xa1: iana(state.c); break;            // ANA C
        case 0xa2: iana(state.d); break;            // ANA D
        case 0xa3: iana(state.e); break;            // ANA E
        case 0xa4: iana(state.h); break;            // ANA H
        case 0xa5: iana(state.l); break;            // ANA L
        case 0xa6: iana(*state.getHLPtr()); break;  // ANA M
        case 0xa7: iana(state.a); break;            // ANA A
    }

    state.pc++;
}

void i8080::unimplementedInstruction() {
    std::cerr << "Error: Unimplemented instruction" << std::endl;

    state.pc--;
    exit(1);
}

void i8080::iadd(uint8_t x) {
    uint16_t ans = static_cast<uint16_t>(state.a) + static_cast<uint16_t>(x);

    state.cc.cy = (ans > 0xff) ? 1 : 0;
    state.a = ans & 0xff;

    handleArithmeticFlags(ans);
}

void i8080::iadc(uint8_t x) {
    uint16_t ans = static_cast<uint16_t>(state.a) + static_cast<uint16_t>(x) +
                   static_cast<uint16_t>(state.cc.cy);

    state.cc.cy = (ans > 0xff) ? 1 : 0;
    uint8_t carry = (state.a & 0x0f) + (x & 0x0f) + (state.cc.cy);
    state.cc.ac = (carry > 0x0f) ? 1 : 0;
    state.a = ans & 0xff;

    handleArithmeticFlags(ans);
}

void i8080::isub(uint8_t x) {
    uint16_t ans = static_cast<uint16_t>(state.a) - static_cast<uint16_t>(x);

    state.cc.cy = (state.a < x) ? 1 : 0;
    state.cc.ac = (~(state.a ^ (ans & 0xff) ^ x) & 0x10) ? 1 : 0;
    state.a = ans & 0xff;

    handleArithmeticFlags(ans);
}

void i8080::isbb(uint8_t x) {
    uint16_t ans =
        static_cast<uint16_t>(state.a) - static_cast<uint16_t>(x + state.cc.cy);

    state.cc.cy = (state.a < static_cast<uint16_t>(x + state.cc.cy)) ? 1 : 0;
    state.cc.ac = (~(state.a ^ (ans & 0xff) ^ x) & 0x10) ? 1 : 0;
    state.a = ans & 0xff;

    handleArithmeticFlags(ans);
}

void i8080::iana(uint8_t x) {
    uint8_t ans = state.a & x;

    state.cc.cy = 0;
    state.cc.ac = ((state.a | x) & 0x08) ? 1 : 0;
    state.a = ans;

    handleArithmeticFlags(ans);
}

void i8080::handleArithmeticFlags(uint16_t ans) {
    state.cc.z = ((ans & 0xff) == 0) ? 1 : 0;
    state.cc.s = ((ans & 0x80) ? 1 : 0);
    state.cc.p = handleParityFlag(ans & 0xff);
}

void i8080::handleArithmeticFlags(uint8_t ans) {
    state.cc.z = (ans == 0) ? 1 : 0;
    state.cc.s = ((ans & 0x80) != 0) ? 1 : 0;
    state.cc.p = handleParityFlag(ans);
}

uint8_t i8080::handleParityFlag(uint8_t ans) {
    return (std::popcount(ans) % 2 == 0) ? 1 : 0;
}
