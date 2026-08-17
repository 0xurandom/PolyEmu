#include "intel8080.hpp"

#include <raylib.h>
#include <sys/types.h>

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

    if (size > 65536) {
        std::cerr << "Error: i8080 rom is too large for memory" << std::endl;
        return false;
    }
    file.seekg(0, std::ios::beg);

    if (!file.read(reinterpret_cast<char*>(state.memory), size)) {
        std::cerr << "Error: Could not read i8080 file" << std::endl;
        return false;
    }

    return true;
}

void i8080::reset() {
    state.pc = 0x0000;
    state.int_enable = 0;

    state.a = 0;
    state.b = 0;
    state.c = 0;
    state.d = 0;
    state.e = 0;
    state.h = 0;
    state.l = 0;

    state.cc.z = 0;
    state.cc.s = 0;
    state.cc.p = 0;
    state.cc.cy = 0;
    state.cc.ac = 0;
}

void i8080::emulate8080() {
    unsigned char* opcode = &state.memory[state.pc];

    uint16_t& pc = state.pc;
    std::cout << "Got opcode: " << std::hex << static_cast<int>(*opcode)
              << std::endl;

    switch (*opcode) {
        // NOP
        case 0x00: {
            pc += 1;
            break;
        }

        case 0x08:
        case 0x10:
        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38: {
            pc += 1;
            break;
        }

        // LXI B, word
        case 0x01: {
            state.c = opcode[1];
            state.b = opcode[2];
            state.pc += 3;
            break;
        }

        // STAX B
        case 0x02: {
            uint16_t adr = state.getBC();
            state.memory[adr] = state.a;
            pc += 1;
            break;
        }

        // INX B
        case 0x03: {
            uint16_t bc = state.getBC();
            bc += 1;
            state.setBC(bc);
            pc += 1;
            break;
        }

        // INR B
        case 0x04: {
            uint8_t ans = state.b + 1;
            handleArithmeticFlags(ans);
            state.b = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR B
        case 0x05: {
            uint8_t ans = state.b - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.b = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // MVI B, D8
        case 0x06: {
            uint8_t ans = state.memory[pc + 1];
            state.b = ans;
            pc += 2;
            break;
        }

        // RLC
        case 0x07: {
            uint8_t x = state.a;
            state.cc.cy = (x >> 7) & 1;
            state.a = (x << 1) | state.cc.cy;
            pc += 1;
            break;
        }

        // DAD B
        case 0x09: {
            uint32_t ans = static_cast<uint32_t>(state.getHL()) +
                           static_cast<uint32_t>(state.getBC());

            state.cc.cy = (ans > 0xffff) ? 1 : 0;
            state.setHL(ans & 0xffff);
            pc += 1;
            break;
        }

        // LDAX B
        case 0x0a: {
            state.a = *state.getBCPtr();
            pc += 1;
            break;
        }

        // DCX B
        case 0x0b: {
            uint16_t bc = state.getBC();
            bc--;
            state.setBC(bc);
            pc += 1;
            break;
        }

        // INR C
        case 0x0c: {
            uint8_t ans = state.c - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.c = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // DCR C
        case 0x0d: {
            uint8_t ans = state.c - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.c = ans;
            handleArithmeticFlags(ans);
            pc += 1;
            break;
        }

        // MVI C, D8
        case 0x0e: {
            uint8_t ans = state.memory[pc + 1];
            state.c = ans;
            pc += 2;
            break;
        }

        // RRC
        case 0x0f: {
            uint8_t x = state.a;
            state.a = ((x & 1) << 7) | (x >> 1);
            state.cc.cy = ((x & 1) == 1);
            pc += 1;
            break;
        }

        // LXI D, D16
        case 0x11: {
            state.e = state.memory[pc + 1];
            state.d = state.memory[pc + 2];
            pc += 3;
            break;
        }

        // STAX D
        case 0x12: {
            uint16_t adr = state.getDE();
            state.memory[adr] = state.a;
            pc += 1;
            break;
        }

        // INX D
        case 0x13: {
            uint16_t de = state.getDE();
            de += 1;
            state.setDE(de);
            pc += 1;
            break;
        }

        // INR D
        case 0x14: {
            uint8_t ans = state.d + 1;
            handleArithmeticFlags(ans);
            state.d = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR D
        case 0x15: {
            uint8_t ans = state.d - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.d = ans;
            handleArithmeticFlags(ans);
            pc += 1;
            break;
        }

        // MVI D, D8
        case 0x16: {
            uint8_t ans = state.memory[pc + 1];
            state.d = ans;
            pc += 2;
            break;
        }

        // RAL
        case 0x17: {
            uint8_t x = state.a;
            state.a = (x << 1) | state.cc.cy;
            state.cc.cy = (x >> 7) & 1;
            pc += 1;
            break;
        }

        // DAD D
        case 0x19: {
            uint32_t ans = static_cast<uint32_t>(state.getHL()) +
                           static_cast<uint32_t>(state.getDE());

            state.cc.cy = (ans > 0xffff) ? 1 : 0;
            state.setHL(ans & 0xffff);
            pc += 1;
            break;
        }

        // LDAX D
        case 0x1a: {
            state.a = *state.getDEPtr();
            pc += 1;
            break;
        }

        // DCX D
        case 0x1b: {
            uint16_t de = state.getDE();
            de--;
            state.setDE(de);
            pc += 1;
            break;
        }

        // INR E
        case 0x1c: {
            uint8_t ans = state.e + 1;
            handleArithmeticFlags(ans);
            state.e = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR E
        case 0x1d: {
            uint8_t ans = state.e - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.e = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // MVI E, D8
        case 0x1e: {
            uint8_t ans = state.memory[pc + 1];
            state.e = ans;
            pc += 2;
            break;
        }

        // RAR
        case 0x1f: {
            uint8_t x = state.a;
            state.a = (state.cc.cy << 7) | (x >> 1);
            state.cc.cy = ((x & 1) == 1);
            pc += 1;
            break;
        }

        // LXI H, D16
        case 0x21: {
            state.l = state.memory[pc + 1];
            state.h = state.memory[pc + 2];

            pc += 3;
            break;
        }

        // SHLD adr
        case 0x22: {
            uint16_t adr = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            state.memory[adr] = state.l;
            state.memory[adr + 1] = state.h;

            pc += 3;
            break;
        }

        // INX H
        case 0x23: {
            uint16_t hl = state.getHL();
            hl += 1;
            state.setHL(hl);
            pc += 1;
            break;
        }

        // INR H
        case 0x24: {
            uint8_t ans = state.h + 1;
            handleArithmeticFlags(ans);
            state.h = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR H
        case 0x25: {
            uint8_t ans = state.h - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.h = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // MVI H, D8
        case 0x26: {
            uint8_t ans = state.memory[pc + 1];
            state.h = ans;
            pc += 2;
            break;
        }

        // DAA
        case 0x27: {
            uint16_t a = state.a;

            if (state.cc.ac || (a & 0x0f) > 9) a += 0x06;
            if (state.cc.cy || (a >> 4) > 9) {
                a += 0x60;
                state.cc.cy = 1;
            }

            state.a = a & 0xff;
            handleArithmeticFlags(state.a);
            pc += 1;

            break;
        }

        // DAD H
        case 0x29: {
            uint32_t ans = static_cast<uint32_t>(state.getHL()) +
                           static_cast<uint32_t>(state.getHL());

            state.cc.cy = (ans > 0xffff) ? 1 : 0;
            state.setHL(ans & 0xffff);
            pc += 1;
            break;
        }

        // LHLD adr
        case 0x2a: {
            uint16_t adr = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            state.l = state.memory[adr];
            state.h = state.memory[adr + 1];
            pc += 3;
            break;
        }

        // DCX H
        case 0x2b: {
            uint16_t hl = state.getHL();
            hl--;
            state.setHL(hl);
            pc += 1;
            break;
        }

        // INR L
        case 0x2c: {
            uint8_t ans = state.l + 1;
            handleArithmeticFlags(ans);
            state.l = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR L
        case 0x2d: {
            uint8_t ans = state.l - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.l = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // MVI L, D8
        case 0x2e: {
            uint8_t ans = state.memory[pc + 1];
            state.l = ans;
            pc += 2;
            break;
        }

        // CMA
        case 0x2f: {
            state.a = ~state.a;
            pc += 1;
            break;
        }

        // LXI SP, D16
        case 0x31: {
            state.sp = (state.memory[pc + 2] << 8) | state.memory[pc + 1];

            pc += 3;
            break;
        }

        // STA adr
        case 0x32: {
            uint16_t adr = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            state.memory[adr] = state.a;

            pc += 3;
            break;
        }

        // INX SP
        case 0x33: {
            state.sp++;
            pc += 1;
            break;
        }

        // INR M
        case 0x34: {
            state.cc.ac = ((*state.getHLPtr() & 0x0f) == 0x0f) ? 1 : 0;
            uint8_t ans = *state.getHLPtr() + 1;
            *state.getHLPtr() = ans;
            handleArithmeticFlags(ans);
            pc += 1;
            break;
        }

        // DCR M
        case 0x35: {
            state.cc.ac = ((*state.getHLPtr() & 0x0f) == 0x0f) ? 1 : 0;
            uint8_t ans = *state.getHLPtr() - 1;
            *state.getHLPtr() = ans;
            handleArithmeticFlags(ans);
            pc += 1;
            break;
        }

        // MVI M, D8
        case 0x36: {
            uint8_t ans = state.memory[pc + 1];
            *state.getHLPtr() = ans;
            pc += 2;
            break;
        }

        // STC
        case 0x37: {
            state.cc.cy = 1;
            pc += 1;
            break;
        }

        case 0x39: {
            uint32_t ans = static_cast<uint32_t>(state.getHL()) +
                           static_cast<uint32_t>(state.sp);
            state.cc.cy = (ans > 0xffff) ? 1 : 0;
            state.setHL(ans & 0xffff);

            pc += 1;
            break;
        }

        // LDA adr
        case 0x3a: {
            uint16_t adr = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            state.a = state.memory[adr];
            pc += 3;
            break;
        }

        // DCX SP
        case 0x3b: {
            state.sp--;
            pc += 1;
            break;
        }

        // INR A
        case 0x3c: {
            uint8_t ans = state.a + 1;
            handleArithmeticFlags(ans);
            state.a = ans;
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR A
        case 0x3d: {
            uint8_t ans = state.a - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
            state.a = ans;
            handleArithmeticFlags(ans);
            state.pc += 1;
            break;
        }

        // MVI A, D8
        case 0x3e: {
            uint8_t ans = state.memory[pc + 1];
            state.a = ans;
            pc += 2;
            break;
        }

        // CMC
        case 0x3f: {
            state.cc.cy = !state.cc.cy;
            break;
        }

        case 0x40:
            state.b = state.b;
            pc += 1;
            break;  // MOV B, B
        case 0x41:
            state.b = state.c;
            pc += 1;
            break;  // MOV B, C
        case 0x42:
            state.b = state.d;
            pc += 1;
            break;  // MOV B, D
        case 0x43:
            state.b = state.e;
            pc += 1;
            break;  // MOV B, E
        case 0x44:
            state.b = state.h;
            pc += 1;
            break;  // MOV B, H
        case 0x45:
            state.b = state.l;
            pc += 1;
            break;  // MOV B, L
        case 0x46:
            state.b = *state.getHLPtr();
            pc += 1;
            break;  // MOV B, M
        case 0x47:
            state.b = state.a;
            pc += 1;
            break;  // MOV B, A
        case 0x48:
            state.c = state.b;
            pc += 1;
            break;  // MOV C, B
        case 0x49:
            state.c = state.c;
            pc += 1;
            break;  // MOV C, C
        case 0x4a:
            state.c = state.d;
            pc += 1;
            break;  // MOV C, D
        case 0x4b:
            state.c = state.e;
            pc += 1;
            break;  // MOV C, E
        case 0x4c:
            state.c = state.h;
            pc += 1;
            break;  // MOV C, H
        case 0x4d:
            state.c = state.l;
            pc += 1;
            break;  // MOV C, L
        case 0x4e:
            state.c = *state.getHLPtr();
            pc += 1;
            break;  // MOV C, M
        case 0x4f:
            state.c = state.a;
            pc += 1;
            break;  // MOV C, A
        case 0x50:
            state.d = state.b;
            pc += 1;
            break;  // MOV D, B
        case 0x51:
            state.d = state.c;
            pc += 1;
            break;  // MOV D, C
        case 0x52:
            state.d = state.d;
            pc += 1;
            break;  // MOV D, D
        case 0x53:
            state.d = state.e;
            pc += 1;
            break;  // MOV D, E
        case 0x54:
            state.d = state.h;
            pc += 1;
            break;  // MOV D, H
        case 0x55:
            state.d = state.l;
            pc += 1;
            break;  // MOV D, L
        case 0x56:
            state.d = *state.getHLPtr();
            pc += 1;
            break;  // MOV D, M
        case 0x57:
            state.d = state.a;
            pc += 1;
            break;  // MOV D, A
        case 0x58:
            state.e = state.b;
            pc += 1;
            break;  // MOV E, B
        case 0x59:
            state.e = state.c;
            pc += 1;
            break;  // MOV E, C
        case 0x5a:
            state.e = state.d;
            pc += 1;
            break;  // MOV E, D
        case 0x5b:
            state.e = state.e;
            pc += 1;
            break;  // MOV E, E
        case 0x5c:
            state.e = state.h;
            pc += 1;
            break;  // MOV E, H
        case 0x5d:
            state.e = state.l;
            pc += 1;
            break;  // MOV E, L
        case 0x5e:
            state.e = *state.getHLPtr();
            pc += 1;
            break;  // MOV E, M
        case 0x5f:
            state.e = state.a;
            pc += 1;
            break;  // MOV E, A
        case 0x60:
            state.h = state.b;
            pc += 1;
            break;  // MOV H, B
        case 0x61:
            state.h = state.c;
            pc += 1;
            break;  // MOV H, C
        case 0x62:
            state.h = state.d;
            pc += 1;
            break;  // MOV H, D
        case 0x63:
            state.h = state.e;
            pc += 1;
            break;  // MOV H, E
        case 0x64:
            state.h = state.h;
            pc += 1;
            break;  // MOV H, H
        case 0x65:
            state.h = state.l;
            pc += 1;
            break;  // MOV H, L
        case 0x66:
            state.h = *state.getHLPtr();
            pc += 1;
            break;  // MOV H, M
        case 0x67:
            state.h = state.a;
            pc += 1;
            break;  // MOV H, A
        case 0x68:
            state.l = state.b;
            pc += 1;
            break;  // MOV L, B
        case 0x69:
            state.l = state.c;
            pc += 1;
            break;  // MOV L, C
        case 0x6a:
            state.l = state.d;
            pc += 1;
            break;  // MOV L, D
        case 0x6b:
            state.l = state.e;
            pc += 1;
            break;  // MOV L, E
        case 0x6c:
            state.l = state.h;
            pc += 1;
            break;  // MOV L, H
        case 0x6d:
            state.l = state.l;
            pc += 1;
            break;  // MOV L, L
        case 0x6e:
            state.l = *state.getHLPtr();
            pc += 1;
            break;  // MOV L, M
        case 0x6f:
            state.l = state.a;
            pc += 1;
            break;  // MOV L, A
        case 0x70:
            *state.getHLPtr() = state.b;
            pc += 1;
            break;  // MOV M, B
        case 0x71:
            *state.getHLPtr() = state.c;
            pc += 1;
            break;  // MOV M, C
        case 0x72:
            *state.getHLPtr() = state.d;
            pc += 1;
            break;  // MOV M, D
        case 0x73:
            *state.getHLPtr() = state.e;
            pc += 1;
            break;  // MOV M, E
        case 0x74:
            *state.getHLPtr() = state.h;
            pc += 1;
            break;  // MOV M, H
        case 0x75:
            *state.getHLPtr() = state.l;
            pc += 1;
            break;         // MOV M, L
        case 0x76: break;  // TODO HLT
        case 0x77:
            *state.getHLPtr() = state.a;
            pc += 1;
            break;  // MOV M, A
        case 0x78:
            state.a = state.b;
            pc += 1;
            break;  // MOV A, B
        case 0x79:
            state.a = state.c;
            pc += 1;
            break;  // MOV A, C
        case 0x7a:
            state.a = state.d;
            pc += 1;
            break;  // MOV A, D
        case 0x7b:
            state.a = state.e;
            pc += 1;
            break;  // MOV A, E
        case 0x7c:
            state.a = state.h;
            pc += 1;
            break;  // MOV A, H
        case 0x7d:
            state.a = state.h;
            pc += 1;
            break;  // MOV A, H
        case 0x7e:
            state.a = *state.getHLPtr();
            pc += 1;
            break;  // MOV A, M
        case 0x7f:
            state.a = state.a;
            pc += 1;
            break;  // MOV A, A

        case 0x80:
            iadd(state.b);
            pc += 1;
            break;  // ADD B
        case 0x81:
            iadd(state.c);
            pc += 1;
            break;  // ADD C
        case 0x82:
            iadd(state.d);
            pc += 1;
            break;  // ADD D
        case 0x83:
            iadd(state.e);
            pc += 1;
            break;  // ADD E
        case 0x84:
            iadd(state.h);
            pc += 1;
            break;  // ADD H
        case 0x85:
            iadd(state.l);
            pc += 1;
            break;  // ADD L
        case 0x86:
            iadd(*state.getHLPtr());
            pc += 1;
            break;  // ADD M
        case 0x87:
            iadd(state.a);
            pc += 1;
            break;  // ADD A

        case 0x88:
            iadc(state.b);
            pc += 1;
            break;  // ADC B
        case 0x89:
            iadc(state.c);
            pc += 1;
            break;  // ADC C
        case 0x8a:
            iadc(state.d);
            pc += 1;
            break;  // ADC D
        case 0x8b:
            iadc(state.e);
            pc += 1;
            break;  // ADC E
        case 0x8c:
            iadc(state.h);
            pc += 1;
            break;  // ADC H
        case 0x8d:
            iadc(state.l);
            pc += 1;
            break;  // ADC L
        case 0x8e:
            iadc(*state.getHLPtr());
            pc += 1;
            break;  // ADC M
        case 0x8f:
            iadc(state.a);
            pc += 1;
            break;  // ADC A

        case 0x90:
            isub(state.b);
            pc += 1;
            break;  // SUB B
        case 0x91:
            isub(state.c);
            pc += 1;
            break;  // SUB C
        case 0x92:
            isub(state.d);
            pc += 1;
            break;  // SUB D
        case 0x93:
            isub(state.e);
            pc += 1;
            break;  // SUB E
        case 0x94:
            isub(state.h);
            pc += 1;
            break;  // SUB H
        case 0x95:
            isub(state.l);
            pc += 1;
            break;  // SUB L
        case 0x96:
            isub(*state.getHLPtr());
            pc += 1;
            break;  // SUB M
        case 0x97:
            isub(state.a);
            pc += 1;
            break;  // SUB A

        case 0x98:
            isbb(state.b);
            pc += 1;
            break;  // SBB B
        case 0x99:
            isbb(state.c);
            pc += 1;
            break;  // SBB C
        case 0x9a:
            isbb(state.d);
            pc += 1;
            break;  // SBB D
        case 0x9b:
            isbb(state.e);
            pc += 1;
            break;  // SBB E
        case 0x9c:
            isbb(state.h);
            pc += 1;
            break;  // SBB H
        case 0x9d:
            isbb(state.l);
            pc += 1;
            break;  // SBB L
        case 0x9e:
            isbb(*state.getHLPtr());
            pc += 1;
            break;  // SBB M
        case 0x9f:
            isbb(state.a);
            pc += 1;
            break;  // SBB A

        case 0xa0:
            iana(state.b);
            pc += 1;
            break;  // ANA B
        case 0xa1:
            iana(state.c);
            pc += 1;
            break;  // ANA C
        case 0xa2:
            iana(state.d);
            pc += 1;
            break;  // ANA D
        case 0xa3:
            iana(state.e);
            pc += 1;
            break;  // ANA E
        case 0xa4:
            iana(state.h);
            pc += 1;
            break;  // ANA H
        case 0xa5:
            iana(state.l);
            pc += 1;
            break;  // ANA L
        case 0xa6:
            iana(*state.getHLPtr());
            pc += 1;
            break;  // ANA M
        case 0xa7:
            iana(state.a);
            pc += 1;
            break;  // ANA A

        case 0xa8:
            ixra(state.b);
            pc += 1;
            break;  // XRA B
        case 0xa9:
            ixra(state.c);
            pc += 1;
            break;  // XRA C
        case 0xaa:
            ixra(state.d);
            pc += 1;
            break;  // XRA D
        case 0xab:
            ixra(state.e);
            pc += 1;
            break;  // XRA E
        case 0xac:
            ixra(state.h);
            pc += 1;
            break;  // XRA H
        case 0xad:
            ixra(state.l);
            pc += 1;
            break;  // XRA L
        case 0xae:
            ixra(*state.getHLPtr());
            pc += 1;
            break;  // XRA M
        case 0xaf:
            ixra(state.a);
            pc += 1;
            break;  // XRA A

        case 0xb0:
            iora(state.b);
            pc += 1;
            break;  // ORA B
        case 0xb1:
            iora(state.c);
            pc += 1;
            break;  // ORA C
        case 0xb2:
            iora(state.d);
            pc += 1;
            break;  // ORA D
        case 0xb3:
            iora(state.e);
            pc += 1;
            break;  // ORA E
        case 0xb4:
            iora(state.h);
            pc += 1;
            break;  // ORA H
        case 0xb5:
            iora(state.l);
            pc += 1;
            break;  // ORA L
        case 0xb6:
            iora(*state.getHLPtr());
            pc += 1;
            break;  // ORA M
        case 0xb7:
            iora(state.a);
            pc += 1;
            break;  // ORA A

        case 0xb8:
            icmp(state.b);
            pc += 1;
            break;  // CMP B
        case 0xb9:
            icmp(state.c);
            pc += 1;
            break;  // CMP C
        case 0xba:
            icmp(state.d);
            pc += 1;
            break;  // CMP D
        case 0xbb:
            icmp(state.e);
            pc += 1;
            break;  // CMP E
        case 0xbc:
            icmp(state.h);
            pc += 1;
            break;  // CMP H
        case 0xbd:
            icmp(state.l);
            pc += 1;
            break;  // CMP L
        case 0xbe:
            icmp(*state.getHLPtr());
            pc += 1;
            break;  // CMP M
        case 0xbf:
            icmp(state.a);
            pc += 1;
            break;  // CMP A

        // RNZ
        case 0xc0: {
            if (state.cc.z == 0) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // POP B
        case 0xc1: {
            state.c = state.memory[state.sp];
            state.b = state.memory[state.sp + 1];
            state.sp += 2;
            pc += 1;
            break;
        }

        // JNZ adr
        case 0xc2: {
            if (state.cc.z == 0)
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            else
                pc += 3;

            break;
        }

        // JMP adr
        case 0xc3: {
            pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            break;
        }

        // CNZ adr
        case 0xc4: {
            if (state.cc.z == 0) {
                uint16_t adr = pc + 3;
                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;
                state.sp -= 2;

                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }

            break;
        }

        // PUSH B
        case 0xc5: {
            state.memory[state.sp - 1] = state.b;
            state.memory[state.sp - 2] = state.c;
            state.sp -= 2;
            pc += 1;
            break;
        }

        // ADI D8
        case 0xc6: {
            iadd(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 0
        case 0xc7: {
            uint16_t adr = state.pc + 1;
            push((adr >> 8) & 0xff, adr & 0xff);
            state.pc = 0x0000;
            break;
        }

        // RZ
        case 0xc8: {
            if (state.cc.z == 1) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // RET
        case 0xc9: {
            pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];

            state.sp += 2;
            break;
        }

        // JZ adr
        case 0xca: {
            if (state.cc.z == 1) {
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        case 0xcb: {
            unimplementedInstruction(*opcode);
            break;
        }

        // CZ adr
        case 0xcc: {
            if (state.cc.z == 1) {
                uint16_t adr = pc + 3;
                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // CALL adr
        case 0xcd: {
            uint16_t adr = pc + 3;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;
            state.sp -= 2;

            pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            break;
        }

        // ACI D8
        case 0xce: {
            iadc(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 1
        case 0xcf: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;
            pc = 0x0008;
            break;
        }

        // RNC
        case 0xd0: {
            if (state.cc.cy == 0) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // POP D
        case 0xd1: {
            state.e = state.memory[state.sp];
            state.d = state.memory[state.sp + 1];
            state.sp += 2;
            pc += 1;
            break;
        }

        // JNC adr
        case 0xd2: {
            if (state.cc.cy == 0) {
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // OUT D8
        case 0xd3: {
            uint8_t port = state.memory[pc + 1];
            machineOut(port, state.a);
            pc += 2;
            break;
        }

        // CNC adr
        case 0xd4: {
            if (state.cc.cy == 0) {
                uint16_t adr = pc + 3;
                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // PUSH D
        case 0xd5: {
            state.memory[state.sp - 1] = state.d;
            state.memory[state.sp - 2] = state.e;
            state.sp -= 2;
            pc += 1;
            break;
        }

        // SUI D8
        case 0xd6: {
            isbb(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 2
        case 0xd7: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;
            pc = 0x0010;
            break;
        }

        // RC
        case 0xd8: {
            if (state.cc.cy == 1) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        case 0xd9: {
            unimplementedInstruction(*opcode);
            break;
        }

        // JC adr
        case 0xda: {
            if (state.cc.cy == 1)
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            else
                pc += 3;
            break;
        }

        // IN D8
        case 0xdb: {
            uint8_t port = state.memory[pc + 1];
            state.a = machineIn(port);
            pc += 2;
            break;
        }

        // CC adr
        case 0xdc: {
            if (state.cc.cy == 1) {
                uint16_t adr = pc + 3;

                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        case 0xdd: {
            // unimplementedInstruction(*opcode);
            pc += 1;
            break;
        }

        // SBI D8
        case 0xde: {
            uint8_t byte = state.memory[pc + 1];
            uint16_t ans = static_cast<uint16_t>(state.a) -
                           static_cast<uint16_t>(byte) -
                           static_cast<uint16_t>(state.cc.cy);
            state.cc.ac =
                (((state.a & 0x0f) - (byte & 0x0f) - state.cc.cy) ? 0 : 1);

            state.cc.cy = (ans > 0xff) ? 1 : 0;
            state.a = ans & 0xff;
            handleArithmeticFlags(state.a);
            pc += 2;
            break;
        }

        // RST 3
        case 0xdf: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;

            pc = 0x0018;
            break;
        }

        // RPO
        case 0xe0: {
            if (state.cc.p == 0) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // POP H
        case 0xe1: {
            state.l = state.memory[state.sp];
            state.h = state.memory[state.sp + 1];
            state.sp += 2;
            pc += 1;
            break;
        }

        // JPO adr
        case 0xe2: {
            if (state.cc.p == 0) {
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // XHTL
        case 0xe3: {
            uint8_t temp1 = state.memory[state.sp];
            uint8_t temp2 = state.memory[state.sp + 1];

            state.memory[state.sp] = state.l;
            state.memory[state.sp + 1] = state.h;

            state.l = temp1;
            state.h = temp2;
            pc += 1;
            break;
        }

        // CPO adr
        case 0xe4: {
            if (state.cc.p == 0) {
                uint16_t adr = pc + 3;
                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;
                state.sp -= 2;
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // PUSH H
        case 0xe5: {
            state.memory[state.sp - 1] = state.h;
            state.memory[state.sp - 2] = state.l;
            state.sp -= 2;
            pc += 1;
            break;
        }

        // ANI D8
        case 0xe6: {
            iana(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 4
        case 0xe7: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;
            pc = 0x0020;

            break;
        }

        // RPE
        case 0xe8: {
            if (state.cc.p == 1) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // PCHL
        case 0xe9: {
            pc = (state.h << 8) | state.l;
            break;
        }

        // JPE adr
        case 0xea: {
            if (state.cc.p == 1)
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            else
                pc += 3;

            break;
        }

        // XCHG
        case 0xeb: {
            uint8_t temp = state.h;
            state.h = state.d;
            state.d = temp;

            temp = state.l;
            state.l = state.e;
            state.e = temp;

            pc += 1;
            break;
        }

        case 0xec: {
            if (state.cc.p == 1) {
                uint16_t adr = pc + 3;

                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;

                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        case 0xed: {
            unimplementedInstruction(*opcode);
            break;
        }

        // XRI D8
        case 0xee: {
            ixra(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 5
        case 0xef: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;

            pc = 0x0028;
            break;
        }

        // RP
        case 0xf0: {
            if (state.cc.s == 0) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // POP PSW
        case 0xf1: {
            state.a = state.memory[state.sp + 1];
            uint8_t psw = state.memory[state.sp];

            state.cc.z = (psw & 0x40) ? 1 : 0;
            state.cc.s = (psw & 0x80) ? 1 : 0;
            state.cc.p = (psw & 0x04) ? 1 : 0;
            state.cc.cy = (psw & 0x01) ? 1 : 0;
            state.cc.ac = (psw & 0x10) ? 1 : 0;

            state.sp += 2;
            pc += 1;
            break;
        }

        // JP adr
        case 0xf2: {
            if (state.cc.s == 0)
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            else
                pc += 3;

            break;
        }

        // DI
        case 0xf3: {
            interruptEnabled = false;
            pc += 1;
            break;
        }

        // CP adr
        case 0xf4: {
            if (state.cc.s == 0) {
                uint16_t adr = pc + 3;

                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;

                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }
            break;
        }

        // PUSH PSW
        case 0xf5: {
            uint8_t psw = 0x02;
            if (state.cc.cy) psw |= 0x01;
            if (state.cc.p) psw |= 0x04;
            if (state.cc.ac) psw |= 0x10;
            if (state.cc.z) psw |= 0x40;
            if (state.cc.s) psw |= 0x80;

            state.memory[state.sp - 1] = state.a;
            state.memory[state.sp - 2] = psw;
            state.sp -= 2;
            pc += 1;
            break;
        }

        // ORI D8
        case 0xf6: {
            iora(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 6
        case 0xf7: {
            uint16_t adr = pc + 1;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            state.sp -= 2;
            pc = 0x0030;
            break;
        }

        // RM
        case 0xf8: {
            if (state.cc.s == 1) {
                pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];
                state.sp += 2;
            } else {
                pc += 1;
            }
            break;
        }

        // SPHL
        case 0xf9: {
            state.sp = (state.h << 8) | state.l;
            pc += 1;
            break;
        }

        // JM adr
        case 0xfa: {
            if (state.cc.s == 1)
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            else
                pc += 3;
            break;
        }

        // EI
        case 0xfb: {
            interruptEnabled = true;
            pc += 1;
            break;
        }

        // CM adr
        case 0xfc: {
            if (state.cc.s == 1) {
                uint16_t adr = pc + 3;

                state.memory[state.sp - 1] = (adr >> 8) & 0xff;
                state.memory[state.sp - 2] = adr & 0xff;

                state.sp -= 2;
                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            } else {
                pc += 3;
            }

            break;
        }

        case 0xfd: {
            unimplementedInstruction(*opcode);
            break;
        }

        // CPI D8
        case 0xfe: {
            icmp(state.memory[pc + 1]);
            pc += 2;
            break;
        }

        // RST 7
        case 0xff: {
            uint16_t adr = pc + 1;
            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;
            state.sp -= 2;
            pc = 0x0038;
            break;
        }
    }
}

void i8080::renderScreen() {
    int pixelCount = 0;
    const uint16_t startAddr = 0x2400;

    for (int offset = 0; offset < 7168; offset++) {
        uint8_t pixelByte = state.memory[startAddr + offset];

        for (int i = 0; i < 8; i++) {
            bool pixel = (pixelByte >> i) & 1;

            int x = offset / 32;
            int y = 255 - ((offset % 32) * 8 + i);

            int bufferAddr = (y * i8080::displayWidth) + x;

            if (pixel)
                screenBuffer[bufferAddr] = 0xFFFFFFFF;
            else
                screenBuffer[bufferAddr] = 0xFF000000;
        }

        pixelCount++;
    }
}

uint8_t i8080::machineIn(uint8_t portNum) {
    uint8_t a = 0;

    switch (portNum) {
        case 0: {
            a = 0x0e;
            break;
        }

        case 1: {
            a = port[1];
            break;
        }

        case 2: {
            a = port[2];
            break;
        }

        case 3: {
            uint16_t v = (shift1 << 8) | shift0;
            a = ((v >> (8 - shiftOffset)) & 0xff);
            break;
        }
    }

    return a;
}

void i8080::machineOut(uint8_t portNum, uint8_t val) {
    switch (portNum) {
        case 2: {
            shiftOffset = val & 0x7;
            break;
        }

        case 4: {
            shift0 = shift1;
            shift1 = val;
            break;
        }
    }
}

void i8080::updateKeys() {
    if (IsKeyDown(KEY_C))
        port[1] |= 0x01;
    else
        port[1] &= ~0x01;
    if (IsKeyDown(KEY_TWO))
        port[1] |= 0x02;
    else
        port[1] &= ~0x02;
    if (IsKeyDown(KEY_ENTER))
        port[1] |= 0x04;
    else
        port[1] &= ~0x04;

    if (IsKeyDown(KEY_SPACE))
        port[1] |= 0x10;
    else
        port[1] &= ~0x01;
    if (IsKeyDown(KEY_LEFT))
        port[1] |= 0x20;
    else
        port[1] &= ~0x20;
    if (IsKeyDown(KEY_RIGHT))
        port[1] |= 0x40;
    else
        port[1] &= ~0x40;

    if (IsKeyDown(KEY_W))
        port[2] |= 0x10;
    else
        port[2] &= ~0x10;
    if (IsKeyDown(KEY_A))
        port[2] |= 0x20;
    else
        port[2] &= ~0x20;
    if (IsKeyDown(KEY_D))
        port[2] |= 0x40;
    else
        port[2] &= ~0x40;
}

void i8080::generateInterrupt(int interruptNum) {
    if (interruptEnabled) {
        push((state.pc & 0xff00) >> 8, (state.pc & 0xff));
        state.pc = 8 * interruptNum;
        interruptEnabled = false;
    }
}

void i8080::push(uint8_t high, uint8_t low) {
    state.memory[state.sp - 1] = high;
    state.memory[state.sp - 2] = low;

    state.sp -= 2;
}

void i8080::unimplementedInstruction(unsigned char opcode) {
    std::cerr << "Error: Unimplemented instruction: " << std::showbase
              << std::hex << static_cast<int>(opcode) << std::endl;

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

void i8080::ixra(uint8_t x) {
    uint8_t ans = state.a ^ x;

    state.cc.cy = 0;
    state.cc.ac = 0;
    state.a = ans;

    handleArithmeticFlags(ans);
}

void i8080::iora(uint8_t x) {
    uint8_t ans = state.a | x;

    state.cc.cy = 0;
    state.cc.ac = 0;
    state.a = ans;
    handleArithmeticFlags(ans);
}

void i8080::icmp(uint8_t x) {
    uint16_t ans = static_cast<uint16_t>(state.a) - static_cast<uint16_t>(x);

    state.cc.cy = (state.a < x) ? 1 : 0;
    state.cc.ac = (~(state.a ^ (ans & 0xff) ^ x) & 0x10) ? 1 : 0;

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
