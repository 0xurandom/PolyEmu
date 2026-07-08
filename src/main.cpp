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

    EmuWindow emuWindow;
    emuWindow.init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Drop ROMs here", 190, 200, 20, LIGHTGRAY);

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
