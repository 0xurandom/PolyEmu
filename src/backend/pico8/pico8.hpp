#pragma once
#include <raylib.h>

#include <cstdint>
#include <lua.hpp>
#include <string>
#include <vector>

#include "../backend.hpp"
#include "api/api.hpp"

class Pico8 : public EmuBackend {
   public:
    Pico8() { reset(); }
    void reset() override;
    static const int displayWidth = 128;
    static const int displayHeight = 128;

    int getDisplayWidth() const override { return displayWidth; }
    int getDisplayHeight() const override { return displayHeight; }

    void renderScreen(std::vector<Color> &pixelBuffer);

    bool loadROM(const std::string &filepath) override;

    void setRawLua(std::string rawLua);

    uint8_t peek(size_t index);
    void poke(size_t index, uint8_t value);

    void processGfxLine(std::string line, size_t GfxLineNum);
    void processMapLine(std::string line, size_t MapLineNum);

   private:
    lua_State *L;
    uint8_t ram[0x8000] = {0};
    std::string RawLua;
    std::string ProcessedLua;

    const uint32_t palette[16] = {
        0x000000FF, 0x1D2B53FF, 0x7E2553FF, 0x008751FF, 0xAB5236FF, 0x5F574FFF,
        0xC2C3C7FF, 0xFFF1E8FF, 0xFF004DFF, 0xFFA300FF, 0xFFEC27FF, 0x00E436FF,
        0x29ADFFFF, 0x83769CFF, 0xFF77A8FF, 0xFFCCAAFF};
};

bool loadCartridge(const std::string &filepath);
