#pragma once

#include <raylib.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include "../backend/chip8/chip8.hpp"

const KeyboardKey Chip8keymap[16] = {
    KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, KEY_Q,    KEY_W, KEY_E, KEY_A,
    KEY_S, KEY_D,   KEY_Z,   KEY_C,     KEY_FOUR, KEY_R, KEY_F, KEY_V,
};

class EmuWindow {
   public:
    void init();
    void initDisplay(int width, int height);
    void updateDisplay(const uint8_t *pixels, int width, int height);
    void drawDisplay();
    void updateKeysPressed();

    void closeEmuWindow();

    int getWidth() { return windowWidth; }
    int getHeight() { return windowHeight; }

    int getMenuBarWidth() { return menuBarWidth; }
    int getMenubarHeight() { return menuBarHeight; }

    void drawMenuBar();

    bool getShowFPS() { return showFPS; }
    void setShowFPS(bool val) { showFPS = val; }

    bool getIsPaused() { return isPaused; }
    void setIsPaused(bool val) { isPaused = val; }

    int getChip8InstPerFrame() { return chip8InstPerFrame; }
    void setchip8InstPerFrame(int val) { chip8InstPerFrame = val; }
    void resetchip8InstPerFrame() { chip8InstPerFrame = defaultInstPerFrame; }

    void openFileDialog();
    void handleROM(const std::string filePath);
    bool getRomIsLoaded() { return romIsLoaded; }
    void setRomIsLoaded(bool val) { romIsLoaded = val; }

    void runEmuFrame();
    void resetEmu();

    void checkKeyboardShortcuts();

    Chip8 *getChip8Ptr() { return chip8; }
    void setChip8Ptr(Chip8 *chip8) { this->chip8 = chip8; }
    std::string gettChip8RomPath() { return chip8RomPath; }

    struct {
        bool fileEditMode = false;
        bool emulatorEditMode = false;

        int fileActive = 0;
        int emulatorActive = 0;
    } menuBar;

   private:
    bool romIsLoaded = false;
    bool isPaused = false;

    bool showFPS = false;
    int targetFPS = 60;

    int scale = 10;

    int windowWidth = 64 * scale;
    int windowHeight = 32 * scale;

    int chip8DisplayWidth = 64;
    int chip8DisplayHeight = 32;

    int menuBarWidth = windowWidth;
    int menuBarHeight = 20;

    static constexpr char Header[] = "PolyEmu";

    // CHIP8
    std::vector<Color> pixelBuffer;
    Texture2D displayTexture{};
    std::string chip8RomPath;

    int chip8InstPerFrame = 11;
    int defaultInstPerFrame = 11;

    Chip8 *chip8 = NULL;
};
