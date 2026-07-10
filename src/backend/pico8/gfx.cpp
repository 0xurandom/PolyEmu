#include <cstdint>

#include "pico8.hpp"
#include "utils.hpp"

using namespace std;

uint8_t hexToInt(char c);

void Pico8::processGfxLine(string line, size_t GfxLineNum) {
    for (size_t i = 0; i < line.length(); i = i + 2) {
        char a = line.at(i);
        char b = line.at(i + 1);

        uint8_t leftNibble = hexToInt(a);
        uint8_t rightNibble = (hexToInt(b)) << 4;

        uint8_t byte = leftNibble | rightNibble;

        size_t index = (GfxLineNum * 64) + (i / 2);

        poke(index, byte);
    }
}
