#pragma once
#include <cstdint>
#include <string>

#include "../backend.hpp"

class Pico8 : public EmuBackend {
   public:
    bool loadROM(const std::string &filepath) override;

    void setRawLua(std::string rawLua);

    uint8_t peek(size_t index);
    void poke(size_t index, uint8_t value);

    void processGfxLine(std::string line, size_t GfxLineNum);
    void processMapLine(std::string line, size_t MapLineNum);

   private:
    uint8_t ram[32768] = {0};
    std::string RawLua;
    std::string ProcessedLua;
};

bool loadCartridge(const std::string &filepath);
