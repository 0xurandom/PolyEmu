#include "frontend.hpp"

#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    //  TODO: autoscale content
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(this->Width, this->Height, this->Header);

    SetTargetFPS(this->targetFPS);
}
int EmuWindow::getWidth() { return Width; }

int EmuWindow::getHeight() { return Height; }
