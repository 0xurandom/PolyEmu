#pragma once
#include <string>

#include "../backend.hpp"

class Pico8 : public EmuBackend {
   public:
    bool loadROM(const std::string &filepath) override;
};

bool loadCartridge(const std::string &filepath);
