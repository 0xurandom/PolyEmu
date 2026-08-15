#pragma once

#include <string>

class EmuBackend {
   public:
    virtual ~EmuBackend() = default;
    virtual bool loadROM(const std::string& filepath) = 0;
    // virtual void runEmuFrame() = 0;

    virtual void reset() = 0;
    virtual int getDisplayWidth() const = 0;
    virtual int getDisplayHeight() const = 0;
};
