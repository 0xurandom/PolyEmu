#include <sys/types.h>

#include <cstdint>

#include "pico8.hpp"
#include "utils.hpp"

using namespace std;

void Pico8::processMapLine(string line, size_t MapLineNum) {
    for (size_t i = 0; i < line.size(); i = i + 2) {
        char a = line.at(i);
        char b = line.at(i + 1);

        uint8_t leftNibble = (hexToInt(a)) << 4;
        uint8_t rightNibble = hexToInt(b);

        uint8_t byte = leftNibble | rightNibble;

        size_t index = 0x2000 + (MapLineNum * 128) + (i / 2);

        poke(index, byte);
    }
}
