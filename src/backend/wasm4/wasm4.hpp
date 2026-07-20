#pragma once

#include <cstdint>
#include <vector>

#include "../backend.hpp"

class Wasm4 : public EmuBackend {
   public:
    bool loadROM(const std::string &filepath) override;

   private:
    std::vector<uint8_t> ram = {0};
};
