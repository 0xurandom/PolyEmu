#include <raylib.h>

#include <iostream>

#include "backend/chip8/chip8.hpp"
#include "frontend/frontend.hpp"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main(int argc, char** argv) {
    Chip8 chip8;

    Vector2 mouseCell = {0.0f, 0.0f};

    EmuWindow emuWindow;
    emuWindow.init();
    emuWindow.initDisplay(Chip8::displayWidth, Chip8::displayHeight);
    emuWindow.setChip8Ptr(&chip8);
    // TODO: init chip8 from window

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

            if (emuWindow.getShowFPS())
                DrawText(TextFormat("fps: %d", GetFPS()), 30, 30, 20, RED);
        } else {
            ClearBackground(LIGHTGRAY);
            DrawText("Drag and drop ROMs here", 200, 200, 20, BLACK);

            GuiGrid(Rectangle{0, (float)emuWindow.getHeight(),
                              (float)emuWindow.getWidth(),
                              (float)emuWindow.getHeight() -
                                  (float)emuWindow.getMenubarHeight()},
                    "grid", (float)20, 1, &mouseCell);
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                std::cout << "Error: Cannot open multiple files" << std::endl;
                exit(1);
            }

            std::string filePath = droppedFiles.paths[0];
            emuWindow.handleROM(filePath);
            UnloadDroppedFiles(droppedFiles);
        }
        emuWindow.drawMenuBar();

        EndDrawing();

        if (emuWindow.getScaleUpdated()) {
            SetWindowSize(emuWindow.getScale() * 64, emuWindow.getScale() * 32);
            emuWindow.setScaleUpdated(false);
        }
    }

    CloseWindow();

    return 0;
}
