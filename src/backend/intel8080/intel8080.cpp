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
            state.pc += 3;
            break;
        }

        // INT B
        case 0x04: {
            uint8_t ans = state.b + 1;
            handleArithmeticFlags(ans);
            state.cc.ac = ((ans & 0x0f) == 0x0f) ? 1 : 0;
            state.pc += 1;
            break;
        }

        // DCR B
        case 0x05: {
            uint8_t ans = state.b - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
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

        // DAD B
        case 0x09: {
            uint32_t ans = static_cast<uint32_t>(state.getHL()) +
                           static_cast<uint32_t>(state.getBC());

            state.cc.cy = (ans > 0xffff) ? 1 : 0;
            state.setHL(ans & 0xffff);
            pc += 1;
            break;
        }

        // DCR C
        case 0x0d: {
            uint8_t ans = state.c - 1;
            state.cc.ac = ((ans & 0x0f) != 0) ? 1 : 0;
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

        // INX D
        case 0x13: {
            uint16_t de = state.getDE();
            de += 1;
            state.setDE(de);
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

        // INX H
        case 0x23: {
            uint16_t hl = state.getHL();
            hl += 1;
            state.setHL(hl);
            pc += 1;
            break;
        }

        // MVI H, D8
        case 0x26: {
            uint8_t ans = state.memory[pc + 1];
            state.h = ans;
            pc += 2;
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

        // MVI M, D8
        case 0x36: {
            uint8_t ans = state.memory[pc + 1];
            state.setHL(ans);
            pc += 2;
        }

        // LDA adr
        case 0x3a: {
            uint16_t adr = (state.memory[pc + 2] << 8) | state.memory[pc + 1];
            state.a = state.memory[adr];
            pc += 3;
            break;
        }

        // MVI A, D8
        case 0x3e: {
            uint8_t ans = state.memory[pc + 1];
            state.a = ans;
            pc += 2;
            break;
        }

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

        case 0xa8: ixra(state.b); break;            // XRA B
        case 0xa9: ixra(state.c); break;            // XRA C
        case 0xaa: ixra(state.d); break;            // XRA D
        case 0xab: ixra(state.e); break;            // XRA E
        case 0xac: ixra(state.h); break;            // XRA H
        case 0xad: ixra(state.l); break;            // XRA L
        case 0xae: ixra(*state.getHLPtr()); break;  // XRA M
        case 0xaf: ixra(state.a); break;            // XRA A

        case 0xb0: iora(state.b); break;            // ORA B
        case 0xb1: iora(state.c); break;            // ORA C
        case 0xb2: iora(state.d); break;            // ORA D
        case 0xb3: iora(state.e); break;            // ORA E
        case 0xb4: iora(state.h); break;            // ORA H
        case 0xb5: iora(state.l); break;            // ORA L
        case 0xb6: iora(*state.getHLPtr()); break;  // ORA M
        case 0xb7: iora(state.a); break;            // ORA A

        case 0xb8: icmp(state.b); break;            // CMP B
        case 0xb9: icmp(state.c); break;            // CMP C
        case 0xba: icmp(state.d); break;            // CMP D
        case 0xbb: icmp(state.e); break;            // CMP E
        case 0xbc: icmp(state.h); break;            // CMP H
        case 0xbd: icmp(state.l); break;            // CMP L
        case 0xbe: icmp(*state.getHLPtr()); break;  // CMP M
        case 0xbf:
            icmp(state.a);
            break;  // CMP A

        // POP B
        case 0xc1: {
            state.c = state.memory[state.sp];
            state.b = state.memory[state.sp + 1];
            state.sp += 2;
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

                pc = (state.memory[pc + 2] << 8) | state.memory[pc + 2];
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
            break;
        }

        // ADI D8
        case 0xc6: {
            uint16_t ans = static_cast<uint16_t>(state.a) +
                           static_cast<uint16_t>(state.memory[pc + 1]);
            handleArithmeticFlags(ans);

            state.cc.cy = (ans > 0xff) ? 1 : 0;
            state.cc.ac =
                (((state.a & 0x0f) + (state.memory[pc + 1])) > 0x0f) ? 1 : 0;
            state.a = ans & 0xff;
            pc += 2;
            break;
        }

        // RET
        case 0xc9: {
            pc = (state.memory[state.sp + 1] << 8) | state.memory[state.sp];

            state.sp += 2;
            break;
        }

        // CALL adr
        case 0xcd: {
            uint16_t adr = pc + 3;

            state.memory[state.sp - 1] = (adr >> 8) & 0xff;
            state.memory[state.sp - 2] = adr & 0xff;

            pc = (state.memory[pc + 2]) | state.memory[pc + 1];
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

        // OUT D8
        case 0xd3: {
            // TODO PORTS
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

        // POP H
        case 0xe1: {
            state.l = state.memory[state.sp];
            state.h = state.memory[state.sp + 1];
            state.sp += 2;
            pc += 2;
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

        // ANI
        case 0xe6: {
            uint8_t ans = state.a & opcode[1];

            state.cc.cy = 0;
            state.a = ans;
            handleArithmeticFlags(ans);
            state.pc++;

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

        // POP PSW
        case 0xf1: {
            state.a = state.memory[state.sp + 1];
            uint8_t psw = state.memory[state.sp];

            state.cc.z = ((psw & 0x01) == 0x01);
            state.cc.s = ((psw & 0x02) == 0x02);
            state.cc.p = ((psw & 0x04) == 0x04);
            state.cc.cy = ((psw & 0x08) == 0x05);
            state.cc.ac = ((psw & 0x10) == 0x10);

            state.sp += 2;
            break;
        }

        // DI
        case 0xf3: {
            interruptEnabled = false;
            pc += 1;
            break;
        }

        // PUSH PSW
        case 0xf5: {
            state.memory[state.sp - 1] = state.a;
            uint8_t psw = (state.cc.z | state.cc.s << 1 | state.cc.p << 2 |
                           state.cc.cy << 3 | state.cc.ac << 4);
            state.memory[state.sp - 1] = psw;
            state.sp -= 2;

            break;
        }

        // EI
        case 0xfb: {
            interruptEnabled = true;
            pc += 1;
            break;
        }

        // CPI
        case 0xfe: {
            uint8_t x = state.a - opcode[1];

            state.cc.cy = (state.a < opcode[1]) ? 1 : 0;
            state.pc++;
            handleArithmeticFlags(x);

            break;
        }

        // RST 7
        case 0xff: {
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

    handleArithmeticFlags(ans);
}

void i8080::icmp(uint8_t x) {
    uint16_t ans = static_cast<uint16_t>(state.a) - static_cast<uint16_t>(x);

    state.cc.cy = (state.a < x) ? 1 : 0;
    state.cc.cy = (~(state.a ^ (ans & 0xff) ^ x) & 0x10) ? 1 : 0;

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
