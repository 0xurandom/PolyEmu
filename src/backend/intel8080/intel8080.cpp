#include "intel8080.hpp"
#include <raylib.h>
#include <fstream>
#include <ios>

bool i8080::loadROM(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    filebuffer.reserve(size);
}