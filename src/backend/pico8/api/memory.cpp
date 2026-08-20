#include <cstddef>
#include <cstdint>
#include <lua.hpp>

#include "../pico8.hpp"

int p_poke(lua_State *L) {
    Pico8 *emu = static_cast<Pico8 *>(lua_touserdata(L, lua_upvalueindex(1)));
    size_t addr = static_cast<size_t>(luaL_checkinteger(L, 1));
    uint8_t val = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    emu->poke(addr, val);
    return 0;
}

int p_peek(lua_State *L) {
    Pico8 *emu = static_cast<Pico8 *>(lua_touserdata(L, lua_upvalueindex(1)));
    size_t addr = static_cast<size_t>(luaL_checkinteger(L, 1));

    emu->peek(addr);
    return 1;
}
