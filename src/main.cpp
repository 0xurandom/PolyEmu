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

        emuWindow.drawMenuBar();

        if (GuiDropdownBox(
                Rectangle{0, 0, 60, (float)emuWindow.getMenubarHeight()},
                "File;Open;Exit", &emuWindow.menuBar.fileActive,
                emuWindow.menuBar.fileEditMode)) {
            emuWindow.menuBar.fileEditMode = !emuWindow.menuBar.fileEditMode;

            switch (emuWindow.menuBar.fileActive) {
                case 1: {
                    emuWindow.openFileDialog();
                    break;
                }

                case 2: {
                    CloseWindow();
                    exit(0);
                }
            }

            emuWindow.menuBar.fileActive = 0;
        }

        if (GuiDropdownBox(
                Rectangle{60, 0, 85, (float)emuWindow.getMenubarHeight()},
                "Emulator;Show FPS;Pause;Increase speed;Decrease speed;Reset "
                "Speed;Reset",
                &emuWindow.menuBar.emulatorActive,
                emuWindow.menuBar.emulatorEditMode)) {
            emuWindow.menuBar.emulatorEditMode =
                !emuWindow.menuBar.emulatorEditMode;

            switch (emuWindow.menuBar.emulatorActive) {
                case 1: {
                    if (emuWindow.getShowFPS())
                        emuWindow.setShowFPS(false);
                    else
                        emuWindow.setShowFPS(true);

                    break;
                }

                case 2: {
                    if (emuWindow.getIsPaused())
                        emuWindow.setIsPaused(false);
                    else
                        emuWindow.setIsPaused(true);
                    break;
                }

                case 3: {
                    int speed = emuWindow.getChip8InstPerFrame();
                    emuWindow.setchip8InstPerFrame(speed + 8);
                    break;
                }

                case 4: {
                    int speed = emuWindow.getChip8InstPerFrame();
                    emuWindow.setchip8InstPerFrame(speed - 8);
                    break;
                }

                case 5: {
                    emuWindow.resetchip8InstPerFrame();
                    break;
                }

                case 6: {
                    emuWindow.resetEmu();
                    break;
                }
            }
            emuWindow.menuBar.emulatorActive = 0;
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
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
