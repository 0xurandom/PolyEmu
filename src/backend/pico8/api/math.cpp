#include <cmath>
#include <cstdint>

#include "api.hpp"

// fixed pt integers go from
// -32768.0 to 32767.99

using namespace std;

int32_t p_abs(int32_t x) {
    if (x < 0)
        return -x;
    else
        return x;
}

int32_t p_atan2(int32_t dx, int32_t dy) {
    // TODO: check if this works
    double x = static_cast<double>(dx);
    double y = static_cast<double>(dy);

    double radians = atan2(-y, x);
    double turns = radians / (2 * M_PI);

    if (turns < 0) turns += 1;
    return static_cast<int32_t>(turns);
}

int32_t p_band(int32_t x, int32_t y) { return x & y; }

int32_t p_bnot(int32_t x) { return ~x; }

int32_t p_bor(int32_t x, int32_t y) { return x | y; }

int32_t p_bxor(int32_t x, int32_t y) { return x ^ y; }

int32_t p_ceil(int32_t x) {
    int32_t fracPart = x & (1 << 16) - 1;
    x -= fracPart;

    if (fracPart == 0)
        return x;
    else
        return x + 1;
}

int32_t p_cos(int32_t x) {
    double turns = static_cast<double>(x);
    return static_cast<int32_t>(cos(turns * 2 * M_PI));
}

int32_t p_flr(int32_t x) {
    int32_t fracPart = x & (1 << 16) - 1;
    x -= fracPart;

    return x;
}

int32_t p_lshr(int32_t n, int32_t bits) {
    int shift = static_cast<int>(n);
    return static_cast<uint32_t>(bits) >> shift;
}

int32_t p_max(int32_t x, int32_t y) {
    if (x > y)
        return x;
    else
        return y;
}

int32_t p_mid(int32_t x, int32_t y, int32_t z) {
    // TODO: allow using this as a clamp
    if ((x <= y && y <= z) || (z <= y && y <= x))
        return y;
    else if ((x <= z && z <= y) || (y <= z && z <= x))
        return z;
    else
        return x;
}

int32_t p_min(int32_t x, int32_t y) {
    if (x < y)
        return x;
    else
        return y;
}

// TODO: this needs arrays as input
int32_t p_rnd() {}

int32_t p_rotl(int32_t n, int32_t bits) {
    int shift = static_cast<int>(bits) & 31;
    return (n << shift) | (static_cast<uint32_t>(n) >> (32 - shift));
}

int32_t p_rotr(int32_t n, int32_t bits) {
    int shift = static_cast<int>(bits) & 31;
    return (n << (32 - shift)) | (static_cast<uint32_t>(n) >> shift);
}

int32_t p_sgn(int32_t x) {
    if (x < 0)
        return -1;
    else
        return 0;
}

int32_t p_shl(int32_t n, int32_t bits) {
    int shift = static_cast<int>(n);
    return bits << shift;
}

int32_t p_shr(int32_t n, int32_t bits) {
    int shift = static_cast<int>(n);
    return bits >> shift;
}

int32_t p_sin(int32_t x) {
    double turns = static_cast<double>(x);
    double radians = turns * 2.0 * M_PI;
    return static_cast<int32_t>(-std::sin(radians));
}

int32_t p_sqrt(int32_t x) {
    if (x < 0)
        return 0;
    else
        return sqrt(x);
}

int32_t p_srand(int32_t x) {
    // TODO: seed rand
}
