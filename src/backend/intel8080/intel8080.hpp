#pragma once

#include <string>
#include <vector>

#include "../backend.hpp"

class i8080 : public EmuBackend {
   public:
    virtual bool loadROM(const std::string &filepath) = 0;

   private:
    std::vector<unsigned char> filebuffer;
};
