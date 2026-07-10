#include "pico8.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

enum CartSection {
    LUA,
    GFX,
    MAP,
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
    CartSection cartSection;

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
        }

        switch (cartSection) {
            case LUA: {
                rawLua += line + '\n';
                break;
            }

            case GFX: {
                processGfxLine(line, gfxStart - lineNum);
            }

            case MAP: {
                processMapLine(line, mapStart - lineNum);
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

uint8_t Pico8::peek(size_t index) { return ram[index]; }
void Pico8::poke(size_t index, uint8_t value) {
    ram[index] = value;
    return;
}
