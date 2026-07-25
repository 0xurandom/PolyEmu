#include <raylib.h>

#include <cstring>
#include <iostream>

#include "backend/chip8/chip8.hpp"
#define RAYGUI_IMPLEMENTATION
#include "frontend/frontend.hpp"
#include "raygui.h"

using namespace std;

int main(int argc, char** argv) {
    Chip8 chip8;

    bool romIsLoaded = false;

    Vector2 mouseCell = {0.0f, 0.0f};

    EmuWindow emuWindow;
    emuWindow.init();

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

        ClearBackground(RAYWHITE);

        if (romIsLoaded) {
            emuWindow.drawDisplay();
        } else {
            DrawText("Drag and drop ROMs here", 200, 200, 20, LIGHTGRAY);
        }

        GuiGrid(Rectangle{0, 30, (float)emuWindow.getWidth(),
                          (float)emuWindow.getHeight() - 20},
                "grid", (float)20, 1, &mouseCell);

        GuiPanel(Rectangle{0, 0, (float)emuWindow.getWidth()}, nullptr);

        if (GuiDropdownBox(Rectangle{0, 0, 75, 20}, "File;Open;Exit",
                           &emuWindow.menuBar.fileActive,
                           emuWindow.menuBar.fileEditMode)) {
        }

        if (GuiDropdownBox(Rectangle{75 + 0, 0, 75 + 75, 20}, "Emulator;Reset",
                           &emuWindow.menuBar.emulatorActive,
                           emuWindow.menuBar.emulatorEditMode)) {
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                cout << "Cannot open multiple files\n";
                exit(1);
            }

            string filePath = droppedFiles.paths[0];
            const char* extension = GetFileExtension(filePath.c_str());

            if (extension != nullptr && strcmp(extension, ".ch8") == 0) {
                if (chip8.loadROM(filePath)) {
                    std::cout << "Successfully loaded chip8 rom\n";
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
