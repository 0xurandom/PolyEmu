#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../backend.hpp"

struct ConditionCodes {
    uint8_t z : 1;
    uint8_t s : 1;
    uint8_t p : 1;
    uint8_t cy : 1;
    uint8_t ac : 1;
    uint8_t pad : 3;
};

struct State8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t h;
    uint8_t e;
    uint8_t l;
    uint16_t sp;
    uint16_t pc;
    uint8_t* memory;
    ConditionCodes cc;
    uint8_t int_enable;

    uint16_t getHL() { return (h << 8) | l; }

    void setHL(uint16_t val) {
        h = (val >> 8) & 0xff;
        l = val & 0xff;
    }

    uint8_t* getHLPtr() { return &memory[getHL()]; }

    uint16_t getBC() { return (b << 8) | c; }

    void setBC(uint16_t val) {
        b = (val >> 8) & 0xff;
        c = val & 0xff;
    }

    uint8_t* getBCPtr() { return &memory[getBC()]; }
};

class i8080 : public EmuBackend {
   public:
    virtual bool loadROM(const std::string& filepath) = 0;

   private:
    int disassemble();
    int pc = 0;
    std::vector<unsigned char> filebuffer;

    State8080 state;

    void emulate8080();

    void unimplementedInstruction();

    void iadd(uint8_t x);
    void iadc(uint8_t x);
    void isub(uint8_t x);
    void isbb(uint8_t x);

    void iana(uint8_t x);
    void ixra(uint8_t x);
    void iora(uint8_t x);
    void icmp(uint8_t x);

    void handleArithmeticFlags(uint16_t ans);
    void handleArithmeticFlags(uint8_t ans);
    uint8_t handleParityFlag(uint8_t ans);
};
