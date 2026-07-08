#pragma once

class EmuWindow {
   public:
    void init();
    int getWidth();
    int getHeight();

    struct {
        bool fileEditMode = false;
        bool emulatorEditMode = false;

        int fileActive = 0;
        int emulatorActive = 0;
    } menuBar;

   private:
    int Width = 854;
    int Height = 480;

    static constexpr char Header[] = "PolyEmu";

    int targetFPS = 60;
};
