#include <raylib.h>

#include <cstring>
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

    while (!WindowShouldClose()) {
        if (emuWindow.getRomIsLoaded()) {
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

        if (emuWindow.getRomIsLoaded()) {
            emuWindow.drawDisplay();

            if (emuWindow.getShowFPS())
                DrawText(TextFormat("fps: %d", GetFPS()), 30, 30, 20, RED);

        } else {
            DrawText("Drag and drop ROMs here", 200, 200, 20, BLACK);

            GuiGrid(Rectangle{0, (float)emuWindow.getHeight(),
                              (float)emuWindow.getWidth(),
                              (float)emuWindow.getHeight() -
                                  (float)emuWindow.getMenubarHeight()},
                    "grid", (float)20, 1, &mouseCell);
        }
        GuiPanel(Rectangle{0, 0, (float)emuWindow.getWidth()}, nullptr);

        if (GuiDropdownBox(
                Rectangle{0, 0, 75, (float)emuWindow.getMenubarHeight()},
                "File;Open;Exit", &emuWindow.menuBar.fileActive,
                emuWindow.menuBar.fileEditMode)) {
            std::cout << "File pressed" << std::endl;
        }

        if (GuiDropdownBox(Rectangle{75 + 0, 0, 75 + 75,
                                     (float)emuWindow.getMenubarHeight()},
                           "Emulator;Reset", &emuWindow.menuBar.emulatorActive,
                           emuWindow.menuBar.emulatorEditMode)) {
            std::cout << "Emulator presed" << std::endl;
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                std::cout << "Error: Cannot open multiple files\n";
                exit(1);
            }

            std::string filePath = droppedFiles.paths[0];
            emuWindow.handleROM(filePath);
            UnloadDroppedFiles(droppedFiles);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
