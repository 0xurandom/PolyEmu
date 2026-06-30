#pragma once

class EmuWindow {
   public:
    void init();

   private:
    int Width = 1280;
    int Height = 720;
    static constexpr char Header[] = "PolyEmu Window";

    int targetFPS = 60;
};
