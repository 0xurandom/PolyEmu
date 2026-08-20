// game loop

// math
#include <lua.h>

#include <cstdint>

int32_t p_abs(int32_t x);
int32_t p_atan2(int32_t dx, int32_t dy);
int32_t p_band(int32_t x, int32_t y);
int32_t p_bnot(int32_t x);
int32_t p_bor(int32_t x, int32_t y);
int32_t p_bxor(int32_t x, int32_t y);
int32_t p_ceil(int32_t x);
int32_t p_cos(int32_t x);
int32_t p_flr(int32_t x);
int32_t p_lshr(int32_t n, int32_t bits);
int32_t p_max(int32_t x, int32_t y = 0);
int32_t p_mid(int32_t x, int32_t y, int32_t z);
int32_t p_min(int32_t x, int32_t y = 0);
int32_t p_rnd();  // not working yet
int32_t p_rotl(int32_t n, int32_t bits);
int32_t p_rotr(int32_t n, int32_t bits);
int32_t p_sgn(int32_t x);
int32_t p_shl(int32_t n, int32_t bits);
int32_t p_shr(int32_t n, int32_t bits);
int32_t p_sin(int32_t x);
int32_t p_sqrt(int32_t x);
int32_t p_srand(int32_t x);  // also not working

int p_poke(lua_State *L);
int p_peek(lua_State *L);
