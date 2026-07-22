#pragma once

#include <raylib.h>

#include <vector>
class EmuWindow {
   public:
    void init();
    void initDisplay();

    int getWidth();
    int getHeight();

    int getMenuBarWidth();
    int getMenubarHeight();

    struct {
        bool fileEditMode = false;
        bool emulatorEditMode = false;

        int fileActive = 0;
        int emulatorActive = 0;
    } menuBar;

   private:
    int Width = 64;
    int Height = 32;

    int scale = 6;

    int menuBarWidth = Width;
    int menuBarHeight = 30;

    static constexpr char Header[] = "PolyEmu";

    int targetFPS = 60;

    std::vector<Color> pixelBuffer;
    Texture2D displayTexture[];
};
