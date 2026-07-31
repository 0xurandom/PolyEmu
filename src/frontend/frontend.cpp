#include "frontend.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nfd.hpp>
#include <sstream>
#include <string>

#include "nativefiledialog-extended/src/include/nfd.h"
#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    //  TODO: autoscale content
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, this->Header);
    SetTargetFPS(this->config.targetFPS);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(WHITE));
}

void EmuWindow::initDisplay(int width, int height) {
    Image img = GenImageColor(width, height, BLACK);

    displayTexture = LoadTextureFromImage(img);

    UnloadImage(img);
    SetTextureFilter(displayTexture, TEXTURE_FILTER_POINT);

    pixelBuffer.resize(width * height);
}

void EmuWindow::updateDisplay(const uint8_t *pixels, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        if (pixels[i])
            pixelBuffer[i] = WHITE;
        else
            pixelBuffer[i] = BLACK;
    }

    UpdateTexture(displayTexture, pixelBuffer.data());
}

void EmuWindow::drawDisplay() {
    Rectangle src = {0, 0, (float)displayTexture.width,
                     (float)displayTexture.height};

    Rectangle dest = {
        0,
        (float)menuBarHeight,
        (float)GetScreenWidth(),
        (float)(GetScreenHeight() - menuBarHeight),
    };

    DrawTexturePro(displayTexture, src, dest, {0, 0}, 0, WHITE);
}

void EmuWindow::updateChip8KeysPressed() {
    for (int i = 0; i < 16; i++) {
        getChip8Ptr()->setKeyState(i, IsKeyDown(Chip8keymap[i]));
    }
}

void EmuWindow::handleROM(const std::string filePath) {
    const char *extension = GetFileExtension(filePath.c_str());

    if (extension != nullptr && strcmp(extension, ".ch8") == 0) {
        if (getRomIsLoaded()) {
            getChip8Ptr()->reset();
        }

        if (getChip8Ptr()->loadROM(filePath)) {
            std::cout << "Chip8: Successfully loaded chip8 rom: " << filePath
                      << std::endl;
            chip8RomPath = filePath;
            setRomIsLoaded(true);
        } else {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
        }
    } else {
        std::cout << "Error: Unknown file" << std::endl;
    }
}

void EmuWindow::runEmuFrame() {
    updateChip8KeysPressed();

    for (int i = 0; i < getChip8InstPerFrame(); i++) {
        Opcode opcode = chip8->getOpcode();
        chip8->handleOpcode(opcode);
    }
    chip8->runTimers();

    updateDisplay(chip8->getDisplay(), Chip8::displayWidth,
                  Chip8::displayHeight);
}

void EmuWindow::openFileDialog() {
    NFD_Init();

    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[2] = {{"Chip8 ROMs", "ch8"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;

    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

    if (result == NFD_OKAY) {
        std::string outPathString(outPath);
        handleROM(outPathString);
    }

    NFD_Quit();
}

void EmuWindow::checkKeyboardShortcuts() {
    // Open: Ctrl + O
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_O)) {
        std::cout << "Window: Ctrl + O pressed" << std::endl;
        openFileDialog();
    }

    // Reset: Ctrl + R
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_R)) {
        std::cout << "Window: Ctrl + R pressed" << std::endl;
        resetEmu();
    }

    return;
}

void EmuWindow::initConfig() {
    std::string configPath = getConfigPath();

    if (configPath == "") {
        std::cerr << "Error: Could not init config due to invalid config path"
                  << std::endl;
        return;
    }

    std::filesystem::path filePath = configPath;

    if (std::filesystem::exists(filePath)) {
        loadConfig(configPath);
    } else {
        saveConfig(configPath);
    }

    return;
}

std::string EmuWindow::getConfigPath() {
    std::string configPath = "";

#if defined(__linux__)
    char *home = std::getenv("HOME");
    if (home != NULL)
        configPath = std::string(home) + "./config/PolyEmu.conf";
    else
        std::cerr << "Error: Could not get home path for linux" << std::endl;

#elif defined(_WIN32)
    char *appdata = std::getenv("APPDATA");

    if (appdata != NULL)
        configPath = std::string(appdata) + "\\PolyEmu.conf";
    else
        std::cerr << "Error: Could not get appdata path for windows"
                  << std::endl;

#else
    std::cerr << "Error: Compiled for unknown OS" << std::endl;

#endif

    return configPath;
}

void EmuWindow::loadConfig(std::string configPath) {
    std::ifstream file(configPath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream lineStream(line);
        std::string var;
        std::string val;

        if (std::getline(lineStream, var, '=')) {
            if (std::getline(lineStream, val)) {
                if (var == "showFPS") {
                    if (val == "1" || val == "true")
                        config.showFPS = true;
                    else
                        config.showFPS = false;

                    std::cout << "Loaded config showFPS: " << config.showFPS
                              << std::endl;

                } else if (var == "targetFPS") {
                    config.targetFPS = std::stoi(val);

                    std::cout << "Loaded config targetFPS: " << config.targetFPS
                              << std::endl;
                } else {
                    std::cerr
                        << "Warning: Unknown variable in config file: " << var
                        << std::endl;
                }
            }
        }
    }

    file.close();
}

void EmuWindow::saveConfig(std::string configPath) {
    std::ofstream file(configPath);

    if (!file.is_open())
        std::cerr << "Error: Could not open config file: " << configPath
                  << std::endl;

    file << "showFPS=" << config.showFPS << '\n';
    file << "targetFPS=" << config.targetFPS << '\n';

    file.close();

    return;
}

void EmuWindow::drawMenuBar() {
    GuiPanel(
        Rectangle{0, 0, (float)GetScreenWidth(), (float)getMenubarHeight()},
        nullptr);

    if (GuiDropdownBox(Rectangle{0, 0, 60, (float)getMenubarHeight()},
                       "File;Open;Exit", &menuBar.fileActive,
                       menuBar.fileEditMode)) {
        menuBar.fileEditMode = !menuBar.fileEditMode;

        std::cout << "file: " << menuBar.fileActive << std::endl;

        switch (menuBar.fileActive) {
            case 1: {
                openFileDialog();
                break;
            }

            case 2: {
                CloseWindow();
                exit(0);
            }
        }

        menuBar.fileActive = 0;
    }

    if (GuiDropdownBox(
            Rectangle{60, 0, 85, (float)getMenubarHeight()},
            "Emulator;Show FPS;Pause;Increase speed;Decrease speed;Reset "
            "Speed;Reset",
            &menuBar.emulatorActive, menuBar.emulatorEditMode)) {
        menuBar.emulatorEditMode = !menuBar.emulatorEditMode;

        switch (menuBar.emulatorActive) {
            case 1: {
                if (getShowFPS())
                    setShowFPS(false);
                else
                    setShowFPS(true);

                break;
            }

            case 2: {
                if (getIsPaused())
                    setIsPaused(false);
                else
                    setIsPaused(true);
                break;
            }

            case 3: {
                int speed = getChip8InstPerFrame();
                setchip8InstPerFrame(speed + 8);
                break;
            }

            case 4: {
                int speed = getChip8InstPerFrame();
                setchip8InstPerFrame(speed - 8);
                break;
            }

            case 5: {
                resetchip8InstPerFrame();
                break;
            }

            case 6: {
                resetEmu();
                break;
            }
        }
        menuBar.emulatorActive = 0;
    }

    if (GuiDropdownBox(Rectangle{60 + 85, 0, 85, (float)getMenubarHeight()},
                       "View;Increase Scale;Decrease Scale",
                       &menuBar.viewActive, menuBar.viewEditMode)) {
        menuBar.viewEditMode = !menuBar.viewEditMode;

        switch (menuBar.viewActive) {
            case 1: {
                int tempScale = getScale();
                setScale(tempScale + 1);

                setScaleUpdated(true);
                break;
            }
            case 2: {
                int tempScale = getScale();
                setScale(tempScale - 1);

                setScaleUpdated(true);
                break;
            }
        }

        menuBar.viewActive = 0;
    }
}

void EmuWindow::resetEmu() {
    getChip8Ptr()->reset();
    getChip8Ptr()->loadROM(gettChip8RomPath());
}

void EmuWindow::closeEmuWindow() {
    UnloadTexture(displayTexture);
    CloseWindow();
}
