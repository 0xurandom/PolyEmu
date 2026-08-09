#pragma once

#include <raylib.h>

#include <cstdint>
#include <cstring>
#include <memory>
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

    void drawSettingsWindow();

    void closeEmuWindow();

    int getWidth() { return windowWidth; }
    int getHeight() { return windowHeight; }

    int getChip8Scale() { return config.chip8Scale; }
    void setChip8Scale(int val) { config.chip8Scale = val; }
    void updateChip8Scale();

    void resizeToRatio();
    void toggleBorderlessWindow();
    void toggleFullscreen();

    int getMenuBarWidth() { return menuBarWidth; }
    int getMenubarHeight() { return menuBarHeight; }

    void drawMenuBar();

    bool getScaleUpdated() { return scaleUpdated; }
    void setScaleUpdated(bool val) { scaleUpdated = val; }

    bool getFullscreenToggle() { return fullscreenToggle; }
    void setFullscreenToggle(bool val) { fullscreenToggle = val; }

    bool getShowFPS() { return config.showFPS; }
    void setShowFPS(bool val) { config.showFPS = val; }

    bool getIsPaused() { return isPaused; }
    void setIsPaused(bool val) { isPaused = val; }
    void toggleIsPaused() { isPaused = !isPaused; }

    bool getInFullscreen() { return inFullscreen; }
    void setInFullscreen(bool val) { inFullscreen = val; }

    bool getInFF() { return inFF; }
    void setInFF(bool val) { inFF = val; }

    bool getSettingsOpened() { return settingsOpened; }
    void setSettingsOpened(bool val) { settingsOpened = val; }

    int getChip8InstPerFrame() { return chip8InstPerFrame; }
    void setchip8InstPerFrame(int val) { chip8InstPerFrame = val; }
    void resetchip8InstPerFrame() { chip8InstPerFrame = defaultInstPerFrame; }

    void openFileDialog();
    std::string getRomName(const std::string filePath);
    void handleROM(const std::string filePath);
    bool getRomIsLoaded() { return romIsLoaded; }
    void setRomIsLoaded(bool val) { romIsLoaded = val; }

    void runEmuFrame();
    void resetEmu();

    void drawGreeting();
    void drawFPS();
    void displayFFIndicator();
    void displayPauseIndicator();
    void displayIndicator(std::string indicator);

    void initConfig();
    std::string getConfigPath();
    void loadConfig(std::string configPath);
    void saveConfig(std::string configPath);

    void checkKeyboardShortcuts();

    Chip8 &getChip8Ptr() { return *chip8; }
    std::string gettChip8RomPath() { return chip8RomPath; }

   private:
    struct {
        bool fileEditMode = false;
        bool emulatorEditMode = false;
        bool viewEditMode = false;

        int fileActive = 0;
        int emulatorActive = 0;
        int viewActive = 0;
    } menuBar;

    struct {
        bool showFPS = false;
        int targetFPS = 60;
        bool rememberWindowSize = false;
        int windowWidth = 0;
        int windowHeight = 0;

        int chip8Scale = 10;
        int chip8InstPerFrame = 11;
    } config;

    struct {
        int width = 0;
        int height = 0;
        Vector2 position = {0.0f, 0.0f};
    } prevWindowState;

    bool romIsLoaded = false;
    bool isPaused = false;
    bool inFF = false;

    // TODO: add esc to exit fullscreen
    bool fullscreenToggle = false;
    bool inFullscreen = false;
    bool settingsOpened = false;

    Vector2 settingsScrollPos = {0, 0};
    Rectangle settingsScrollView = {0};

    const int chip8DisplayWidth = 64;
    const int chip8DisplayHeight = 32;

    const int chip8FFincrement = 15;

    int windowWidth = chip8DisplayWidth * config.chip8Scale;
    int windowHeight = chip8DisplayHeight * config.chip8Scale;

    int menuBarWidth = windowWidth;
    int menuBarHeight = 20;

    static constexpr char Header[] = "PolyEmu";

    // CHIP8

    bool scaleUpdated = false;

    void updateChip8KeysPressed();

    std::vector<Color> pixelBuffer;
    Texture2D displayTexture{};
    std::string chip8RomPath;

    int chip8InstPerFrame = 11;
    int defaultInstPerFrame = 11;

    std::unique_ptr<Chip8> chip8 = std::make_unique<Chip8>();
};
