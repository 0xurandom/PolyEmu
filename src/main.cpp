#include <raylib.h>

#include <iostream>
#define RAYGUI_IMPLEMENTATION
#include "backend/pico8/pico8.hpp"
#include "frontend/frontend.hpp"
#include "raygui.h"

// TODO: use cmake to compile
// raylib

using namespace std;

int main(int argc, char** argv) {
    cout << "hello world!\n";

    Pico8 pico8;

    Vector2 mouseCell = {0.0f, 0.0f};

    EmuWindow emuWindow;
    emuWindow.init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        GuiGrid(Rectangle{0, 30, (float)emuWindow.getWidth(),
                          (float)emuWindow.getHeight() - 30},
                "grid", (float)20, 1, &mouseCell);
        DrawText("Drag and drop ROMs here", 200, 200, 20, LIGHTGRAY);

        GuiPanel(Rectangle{0, 0, (float)emuWindow.getWidth()}, nullptr);

        if (GuiDropdownBox(Rectangle{0, 0, 600, 30}, "File;Open;Exit",
                           &emuWindow.menuBar.fileActive,
                           emuWindow.menuBar.fileEditMode)) {
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 1) {
                cout << "Cannot open multiple files\n";
                exit(1);
            }

            string filePath = droppedFiles.paths[0];
            pico8.loadROM(filePath);
            UnloadDroppedFiles(droppedFiles);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
