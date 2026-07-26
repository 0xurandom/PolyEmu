#include <raylib.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#include "backend/chip8/chip8.hpp"
#define RAYGUI_IMPLEMENTATION
#include "frontend/frontend.hpp"
#include "raygui.h"

#define SHOW_FPS 1

int main(int argc, char** argv) {
    Chip8 chip8;

    bool romIsLoaded = false;

    Vector2 mouseCell = {0.0f, 0.0f};

    EmuWindow emuWindow;
    emuWindow.init();
    emuWindow.initDisplay(Chip8::displayWidth, Chip8::displayHeight);

    while (!WindowShouldClose()) {
        if (romIsLoaded) {
            emuWindow.updateKeysPressed(chip8);

            // run 11 instructions per frame
            for (int i = 0; i < 11; i++) {
                Opcode opcode = chip8.getOpcode();
                chip8.handleOpcode(opcode);
            }
            chip8.runTimers();

            emuWindow.updateDisplay(chip8.getDisplay(), Chip8::displayWidth,
                                    Chip8::displayHeight);
        }

        BeginDrawing();

        ClearBackground(LIGHTGRAY);

        if (romIsLoaded) {
            emuWindow.drawDisplay();

            DrawText(TextFormat("fps: %d", GetFPS()), 30, 30, 10, RED);

        } else {
            DrawText("Drag and drop ROMs here", 200, 200, 20, LIGHTGRAY);

            GuiGrid(Rectangle{0, (float)emuWindow.getHeight(),
                              (float)emuWindow.getWidth(),
                              (float)emuWindow.getHeight() - 20},
                    "grid", (float)20, 1, &mouseCell);
        }
        GuiPanel(Rectangle{0, 0, (float)emuWindow.getWidth()}, nullptr);

        if (GuiDropdownBox(
                Rectangle{0, 0, 75, (float)emuWindow.getMenubarHeight()},
                "File;Open;Exit", &emuWindow.menuBar.fileActive,
                emuWindow.menuBar.fileEditMode)) {
        }

        if (GuiDropdownBox(Rectangle{75 + 0, 0, 75 + 75,
                                     (float)emuWindow.getMenubarHeight()},
                           "Emulator;Reset", &emuWindow.menuBar.emulatorActive,
                           emuWindow.menuBar.emulatorEditMode)) {
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                std::cout << "Cannot open multiple files\n";
                exit(1);
            }

            std::string filePath = droppedFiles.paths[0];
            const char* extension = GetFileExtension(filePath.c_str());

            if (extension != nullptr && strcmp(extension, ".ch8") == 0) {
                if (chip8.loadROM(filePath)) {
                    std::cout << "Chip8: Successfully loaded chip8 rom\n";
                    romIsLoaded = true;
                } else {
                    std::cerr << "Error: Couldn't load chip8 rom\n";
                }
            } else {
                std::cout << "Error: Unknown file\n";
            }

            UnloadDroppedFiles(droppedFiles);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
