#pragma once
#include <cstdint>
#include <string>

#include "../backend.hpp"

class Pico8 : public EmuBackend {
   public:
    bool loadROM(const std::string &filepath) override;
    uint8_t peek(size_t index);
    void poke(size_t index, uint8_t value);

   private:
    uint8_t ram[32768] = {0};
};

bool loadCartridge(const std::string &filepath);
