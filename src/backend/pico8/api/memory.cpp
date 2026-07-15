#include <cstdint>

#include "../pico8.hpp"

int p_peek(uint16_t addr, Pico8 pico8, uint8_t amount = 1) {
    return pico8.peek(addr);
}
