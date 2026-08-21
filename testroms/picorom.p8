pico-8 cartridge
version 32
__lua__
function _init()
  for i=0, 8191 do

    local row = flr(i // 64)
    local col = i % 64

    local color_left = (row + col) % 16
    local color_right = (row + col + 1) % 16

    local val = bor(color_left, shl(color_right, 4))

    poke(24576 + i, val)
  end
end
__gfx__
__gff__
__map__
__sfx__
__music__
