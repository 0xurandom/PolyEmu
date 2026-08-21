#include "pico8.hpp"

#include <lauxlib.h>
#include <lua.h>
#include <raylib.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <lua.hpp>
#include <string>

#include "utils.hpp"

using namespace std;

enum CartSection {
    LUA,
    GFX,
    MAP,
    NONE,
};

void Pico8::reset() {
    std::memset(ram, 0, sizeof(ram));
    if (L != nullptr) {
        lua_close(L);
    }

    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, [](lua_State *L) -> int {
        int32_t x = luaL_checkinteger(L, 1);
        lua_pushinteger(L, p_abs(x));
        return 1;
    });
    lua_setglobal(L, "abs");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_flr(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "flr");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_sin(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "sin");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_cos(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "cos");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        int32_t dx = luaL_checkinteger(L, 1);
        int32_t dy = luaL_checkinteger(L, 2);
        lua_pushinteger(L, p_atan2(dx, dy));
        return 1;
    });
    lua_setglobal(L, "atan2");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_band(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "band");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_bor(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "bor");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_bxor(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "bxor");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_shl(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "shl");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_shr(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "shr");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_lshr(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "lshr");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(
            L, p_rotl(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
        return 1;
    });
    lua_setglobal(L, "rotr");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        int32_t x = luaL_checkinteger(L, 1);
        int32_t y = luaL_optinteger(L, 2, 0);
        lua_pushinteger(L, p_max(x, y));
        return 1;
    });
    lua_setglobal(L, "max");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        int32_t x = luaL_checkinteger(L, 1);
        int32_t y = luaL_optinteger(L, 2, 0);
        lua_pushinteger(L, p_min(x, y));
        return 1;
    });
    lua_setglobal(L, "min");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        int32_t x = luaL_checkinteger(L, 1);
        int32_t y = luaL_checkinteger(L, 2);
        int32_t z = luaL_checkinteger(L, 3);
        lua_pushinteger(L, p_mid(x, y, z));
        return 1;
    });
    lua_setglobal(L, "mid");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_ceil(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "ceil");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_bnot(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "bnot");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_sgn(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "sgn");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_sqrt(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "sqrt");

    lua_pushcfunction(L, [](lua_State *L) -> int {
        lua_pushinteger(L, p_srand(luaL_checkinteger(L, 1)));
        return 1;
    });
    lua_setglobal(L, "srand");

    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, p_poke, 1);
    lua_setglobal(L, "poke");

    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, p_peek, 1);
    lua_setglobal(L, "peek");
}

void Pico8::renderScreen(std::vector<Color> &pixelBuffer) {
    const uint16_t vramStart = 0x6000;

    for (int i = 0; i < 8192; i++) {
        uint8_t byte = ram[vramStart + i];

        uint8_t lPixel = byte & 0x0f;
        uint8_t rPixel = (byte >> 4) & 0x0f;

        int bufferAddr = i * 2;
        pixelBuffer[bufferAddr] = GetColor(palette[lPixel]);
        pixelBuffer[bufferAddr + 1] = GetColor(palette[rPixel]);
    }
}

bool Pico8::loadROM(const std::string &filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        cerr << "Could not open file\n";
        return false;
    }

    std::string line;
    size_t lineNum = 1;

    size_t luaStart, gfxStart, mapStart;

    std::string rawLua = "";
    CartSection cartSection = NONE;

    while (getline(file, line)) {
        if (line == "__lua__") {
            luaStart = lineNum;
            cartSection = LUA;
            continue;

        } else if (line == "__gfx__") {
            gfxStart = lineNum;
            cartSection = GFX;
            continue;

        } else if (line == "__map__") {
            mapStart = lineNum;
            cartSection = MAP;
            continue;
        } else if (line.rfind("__", 0) == 0) {
            cerr << "Error: Unexpected ine starting with __: " << line << '\n';
            continue;
        }

        switch (cartSection) {
            case LUA: {
                rawLua += line + '\n';
                break;
            }

            case GFX: {
                processGfxLine(line, lineNum - gfxStart - 1);
                break;
            }

            case MAP: {
                processMapLine(line, lineNum - mapStart - 1);
                break;
            }
            case NONE: {
                break;
            }
        }

        lineNum++;
    }

    file.close();

    if (luaL_dostring(L, rawLua.c_str()) != LUA_OK) {
        std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }

    lua_getglobal(L, "_init");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "Lua _init error: " << lua_tostring(L, -1)
                      << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    cout << "Successfully loaded file\n";
    cout << "Lua:" << rawLua << '\n';
    return true;
}

void Pico8::setRawLua(string rawLua) { RawLua = rawLua; }

uint8_t Pico8::peek(size_t index) { return ram[index]; }
void Pico8::poke(size_t index, uint8_t value) {
    ram[index] = value;
    return;
}

void Pico8::processGfxLine(string line, size_t GfxLineNum) {
    for (size_t i = 0; i + 1 < line.length(); i = i + 2) {
        char a = line.at(i);
        char b = line.at(i + 1);

        uint8_t leftNibble = hexToInt(a);
        uint8_t rightNibble = (hexToInt(b)) << 4;

        uint8_t byte = leftNibble | rightNibble;

        size_t index = (GfxLineNum * 64) + (i / 2);

        poke(index, byte);
    }
}

void Pico8::processMapLine(string line, size_t MapLineNum) {
    for (size_t i = 0; i + 1 < line.size(); i = i + 2) {
        char a = line.at(i);
        char b = line.at(i + 1);

        uint8_t leftNibble = (hexToInt(a)) << 4;
        uint8_t rightNibble = hexToInt(b);

        uint8_t byte = leftNibble | rightNibble;

        size_t index = 0x2000 + (MapLineNum * 128) + (i / 2);

        poke(index, byte);
    }
}
