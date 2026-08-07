#include <raylib.h>

#include <iostream>

#include "backend/chip8/chip8.hpp"
#include "frontend/frontend.hpp"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// TODO: init chip8 from window
// using unique ptr

int main(int argc, char** argv) {
    Chip8 chip8;

    Vector2 mouseCell = {0.0f, 0.0f};

    EmuWindow emuWindow;
    emuWindow.init();
    emuWindow.initDisplay(Chip8::displayWidth, Chip8::displayHeight);
    emuWindow.setChip8Ptr(&chip8);

    if (argc > 1) {
        if (argc != 2)
            std::cerr << "Warning: Cannot open multiple files" << std::endl;

        emuWindow.handleROM(argv[1]);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();

        emuWindow.checkKeyboardShortcuts();

        if (emuWindow.getRomIsLoaded() && (!emuWindow.getIsPaused())) {
            emuWindow.runEmuFrame();
        }

        if (emuWindow.getRomIsLoaded()) {
            emuWindow.drawDisplay();

            if (emuWindow.getShowFPS()) emuWindow.drawFPS();

            if (emuWindow.getIsPaused()) emuWindow.displayPauseIndicator();

            if (emuWindow.getInFF()) emuWindow.displayFFIndicator();

        } else {
            ClearBackground(LIGHTGRAY);

            emuWindow.drawGreeting();
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                std::cout << "Warning: Cannot open multiple files" << std::endl;
            }

            std::string filePath = droppedFiles.paths[0];
            emuWindow.handleROM(filePath);
            UnloadDroppedFiles(droppedFiles);
        }
        emuWindow.drawMenuBar();

        if (emuWindow.getSettingsOpened()) emuWindow.drawSettingsWindow();

        EndDrawing();

        if (emuWindow.getScaleUpdated()) emuWindow.updateChip8Scale();
        if (emuWindow.getFullscreenToggle()) emuWindow.toggleFullscreen();
    }

    emuWindow.saveConfig(emuWindow.getConfigPath());

    CloseWindow();

    return 0;
}
