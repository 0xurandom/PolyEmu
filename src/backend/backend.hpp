#pragma once

#include <string>

class EmuBackend {
   public:
    virtual ~EmuBackend() = default;

    virtual bool loadROM(const std::string &filepath) = 0;

    EmuBackend(int width, int height)
        : displayWidth(width), displayHeight(height) {}

    const int displayWidth = 0;
    const int displayHeight = 0;
};
