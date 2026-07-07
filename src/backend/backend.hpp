#pragma once

#include <string>

class EmuBackend {
   public:
    virtual ~EmuBackend() = default;

    virtual bool loadROM(const std::string &filepath) = 0;
};
