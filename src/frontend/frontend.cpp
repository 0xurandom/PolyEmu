#include "frontend.hpp"

#include <cstdint>

#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    //  TODO: autoscale content
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, this->Header);
    SetTargetFPS(this->targetFPS);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(GRAY));
}

void EmuWindow::initDisplay(int width, int height) {
    Image img = GenImageColor(width, height, BLACK);

    displayTexture = LoadTextureFromImage(img);

    UnloadImage(img);
    SetTextureFilter(displayTexture, TEXTURE_FILTER_POINT);

    pixelBuffer.resize(width * height);
}

void EmuWindow::updateDisplay(const uint8_t *pixels, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        if (pixels[i])
            pixelBuffer[i] = WHITE;
        else
            pixelBuffer[i] = BLACK;
    }

    UpdateTexture(displayTexture, pixelBuffer.data());
}

void EmuWindow::drawDisplay() {
    Rectangle src = {0, 0, (float)displayTexture.width,
                     (float)displayTexture.height};

    Rectangle dest = {
        0,
        (float)menuBarHeight,
        (float)windowWidth,
        (float)(windowHeight - menuBarHeight),
    };

    DrawTexturePro(displayTexture, src, dest, {0, 0}, 0, WHITE);
}

void EmuWindow::updateKeysPressed(Chip8 &chip8) {
    for (int i = 0; i < 16; i++) {
        chip8.setKeyState(i, IsKeyDown(Chip8keymap[i]));
    }
}

void EmuWindow::handleROM(std::string filePath) {
    const char *extension = GetFileExtension(filePath.c_str());

    if (extension != nullptr && strcmp(extension, ".ch8") == 0) {
        if (getRomIsLoaded()) {
            getChip8Ptr()->reset();
        }

        if (getChip8Ptr()->loadROM(filePath)) {
            std::cout << "Chip8: Successfully loaded chip8 rom: " << filePath
                      << std::endl;
            setRomIsLoaded(true);
        } else {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
        }
    } else {
        std::cout << "Error: Unknown file" << std::endl;
    }
}

void EmuWindow::closeEmuWindow() {
    UnloadTexture(displayTexture);
    CloseWindow();
}

int EmuWindow::getWidth() { return windowWidth; }

int EmuWindow::getHeight() { return windowHeight; }

int EmuWindow::getMenuBarWidth() { return menuBarWidth; }

int EmuWindow::getMenubarHeight() { return menuBarHeight; }
