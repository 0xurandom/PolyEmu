#include "frontend.hpp"

#include "raylib.h"

void EmuWindow::init() {
    InitWindow(this->Width, this->Height, this->Header);

    SetTargetFPS(this->targetFPS);
}
