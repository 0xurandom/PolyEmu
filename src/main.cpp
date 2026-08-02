#include <raylib.h>

#include <iostream>

#include "backend/chip8/chip8.hpp"
#include "frontend/frontend.hpp"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// TODO: init chip8 from window

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

            if (emuWindow.getShowFPS())
                DrawText(TextFormat("fps: %d", GetFPS()), 30, 30, 20, RED);
        } else {
            ClearBackground(WHITE);

            std::string greetingText =
                "Drag and drop ROMs here\n\n"
                "             or\n\n"
                "  Press Ctrl + O to open\n\n";

            double fontSize = 20.0f;

            Vector2 textSize = MeasureTextEx(
                GetFontDefault(), greetingText.c_str(), fontSize, 2.0f);

            DrawTextEx(
                GetFontDefault(), greetingText.c_str(),
                Vector2{
                    static_cast<float>(GetScreenWidth()) / 2 - (textSize.x / 2),
                    (static_cast<float>(GetScreenHeight()) -
                     emuWindow.getMenubarHeight()) /
                            2 -
                        (textSize.y / 2),
                },
                fontSize, 2.0f, BLACK);

            GuiGrid(
                Rectangle{0, (float)GetScreenHeight(), (float)GetScreenWidth(),
                          (float)GetScreenHeight() -
                              (float)emuWindow.getMenubarHeight()},
                "grid", (float)20, 1, &mouseCell);
        }

        emuWindow.displayIndicator("hello");

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

        EndDrawing();

        if (emuWindow.getScaleUpdated()) emuWindow.updateChip8Scale();
    }

    CloseWindow();

    return 0;
}
