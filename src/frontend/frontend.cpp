#include "frontend.hpp"

#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    //  TODO: autoscale content
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(this->Width * scale, this->Height * scale, this->Header);

    SetTargetFPS(this->targetFPS);
}

void EmuWindow::initDisplay() {}

int EmuWindow::getWidth() { return Width; }

int EmuWindow::getHeight() { return Height; }

int EmuWindow::getMenuBarWidth() { return menuBarWidth; }

int EmuWindow::getMenubarHeight() { return menuBarHeight; }
