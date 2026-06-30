#pragma once

class EmuWindow {
   public:
    void init();

   private:
    int Width = 854;
    int Height = 480;
    static constexpr char Header[] = "PolyEmu Window";

    int targetFPS = 60;
};
