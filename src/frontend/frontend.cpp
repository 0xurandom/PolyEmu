#include "frontend.hpp"

#include <cstdint>
#include <cstdio>
#include <iterator>
#include <nfd.hpp>

#include "nativefiledialog-extended/src/include/nfd.h"
#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    //  TODO: autoscale content
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, this->Header);
    SetTargetFPS(this->targetFPS);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(WHITE));
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

void EmuWindow::updateKeysPressed() {
    for (int i = 0; i < 16; i++) {
        getChip8Ptr()->setKeyState(i, IsKeyDown(Chip8keymap[i]));
    }
}

void EmuWindow::handleROM(const std::string filePath) {
    const char *extension = GetFileExtension(filePath.c_str());

    if (extension != nullptr && strcmp(extension, ".ch8") == 0) {
        if (getRomIsLoaded()) {
            getChip8Ptr()->reset();
        }

        if (getChip8Ptr()->loadROM(filePath)) {
            std::cout << "Chip8: Successfully loaded chip8 rom: " << filePath
                      << std::endl;
            chip8RomPath = filePath;
            setRomIsLoaded(true);
        } else {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
        }
    } else {
        std::cout << "Error: Unknown file" << std::endl;
    }
}

void EmuWindow::runEmuFrame() {
    updateKeysPressed();

    for (int i = 0; i < getChip8InstPerFrame(); i++) {
        Opcode opcode = chip8->getOpcode();
        chip8->handleOpcode(opcode);
    }
    chip8->runTimers();

    updateDisplay(chip8->getDisplay(), Chip8::displayWidth,
                  Chip8::displayHeight);
}

void EmuWindow::openFileDialog() {
    NFD_Init();

    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[2] = {{"Chip8 ROMs", "ch8"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;

    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

    if (result == NFD_OKAY) {
        std::string outPathString(outPath);
        handleROM(outPathString);
    }

    NFD_Quit();
}

void EmuWindow::checkKeyboardShortcuts() {
    // Open: Ctrl + O
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_O)) {
        std::cout << "Window: Ctrl + O pressed" << std::endl;
        openFileDialog();
    }

    // Reset: Ctrl + R
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_R)) {
        std::cout << "Window: Ctrl + R pressed" << std::endl;
        resetEmu();
    }
}

// TODO: moving menu bar buttons here does not work
void EmuWindow::drawMenuBar() {
    GuiPanel(Rectangle{0, 0, (float)getWidth(), (float)getMenubarHeight()},
             nullptr);
}

void EmuWindow::resetEmu() {
    getChip8Ptr()->reset();
    getChip8Ptr()->loadROM(gettChip8RomPath());
}

void EmuWindow::closeEmuWindow() {
    UnloadTexture(displayTexture);
    CloseWindow();
}
