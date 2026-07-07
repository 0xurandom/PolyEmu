#include "frontend.hpp"

#include "raylib.h"
#include "raylib.hpp"

void EmuWindow::init() {
    InitWindow(this->Width, this->Height, this->Header);

    SetTargetFPS(this->targetFPS);
}
