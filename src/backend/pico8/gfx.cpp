#include "gfx.hpp"

#include "pico8.hpp"
using namespace std;

void Pico8::processGfxLine(string line, size_t GfxLineNum) {
    for (size_t i = 0; i < line.length(); i = i + 2) {
        char a = line.at(i);
        char b = line.at(i + 1);

        uint8_t byte = 0;

        uint8_t leftNibble = hexToInt(a);
        uint8_t rightNibble = (hexToInt(b)) << 4;

        byte = leftNibble | rightNibble;

        size_t index = (GfxLineNum * 64) + (i / 2);

        poke(index, byte);
    }
}

uint8_t hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
