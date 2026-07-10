#include "pico8.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "utils.hpp"

using namespace std;

enum CartSection {
    LUA,
    GFX,
    MAP,
    NONE,
};

bool Pico8::loadROM(const string &filepath) {
    ifstream file(filepath);

    if (!file.is_open()) {
        cerr << "Could not open file\n";
        return false;
    }

    string line;
    size_t lineNum = 1;

    size_t luaStart, gfxStart, mapStart;

    string rawLua = "";
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
