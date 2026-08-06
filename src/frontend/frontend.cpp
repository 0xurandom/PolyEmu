#include "frontend.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nfd.hpp>
#include <sstream>
#include <string>

#include "nativefiledialog-extended/src/include/nfd.h"
#include "raygui.h"
#include "raylib.h"

void EmuWindow::init() {
    initConfig();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth;
    int windowHeight;

    if (config.rememberWindowSize && (config.windowWidth != 0) &&
        (config.windowHeight != 0)) {
        windowWidth = config.windowWidth;
        windowHeight = config.windowHeight;
    } else {
        windowWidth = chip8DisplayWidth * config.chip8Scale;
        windowHeight =
            chip8DisplayHeight * config.chip8Scale + getMenubarHeight();
    }

    InitWindow(windowWidth, windowHeight, this->Header);
    SetExitKey(0);
    SetWindowMinSize(64 * 8, (32 + menuBarHeight) * 8);
    SetTargetFPS(this->config.targetFPS);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(WHITE));
    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
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
    float availableWidth = static_cast<float>(GetScreenWidth());
    float availableHeight =
        static_cast<float>(GetScreenHeight() - getMenubarHeight());

    DrawRectangle(0, getMenubarHeight(), availableWidth, availableHeight,
                  DARKGRAY);

    float gameWidth = static_cast<float>(displayTexture.width);
    float gameHeight = static_cast<float>(displayTexture.height);

    float scaleX = availableWidth / gameWidth;
    float scaleY = availableHeight / gameHeight;

    float scale = 0;
    if (scaleX < scaleY)
        scale = scaleX;
    else
        scale = scaleY;

    float scaledWidth = gameWidth * scale;
    float scaledHeight = gameHeight * scale;

    float offsetX = (availableWidth - scaledWidth) / 2.0f;
    float offsetY =
        getMenubarHeight() + ((availableHeight - scaledHeight) / 2.0f);

    Rectangle src = {0, 0, gameWidth, gameHeight};
    Rectangle dest = {offsetX, offsetY, scaledWidth, scaledHeight};

    DrawTexturePro(displayTexture, src, dest, {0, 0}, 0, WHITE);
}

void EmuWindow::updateChip8KeysPressed() {
    for (int i = 0; i < 16; i++) {
        getChip8Ptr()->setKeyState(i, IsKeyDown(Chip8keymap[i]));
    }
}

std::string EmuWindow::getRomName(const std::string filePath) {
    int lastSlashIndex = filePath.rfind('/');
    int fileExtensionIndex = filePath.rfind('.');

    return filePath.substr(lastSlashIndex + 1,
                           (fileExtensionIndex - lastSlashIndex - 1));
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
            SetWindowTitle(getRomName(filePath).c_str());
        } else {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
        }
    } else {
        std::cout << "Error: Unknown file" << std::endl;
    }
}

void EmuWindow::runEmuFrame() {
    updateChip8KeysPressed();

    int speed;

    if (inFF)
        speed = getChip8InstPerFrame() + chip8FFincrement;
    else
        speed = getChip8InstPerFrame();

    for (int i = 0; i < speed; i++) {
        Opcode opcode = chip8->getOpcode();
        chip8->handleOpcode(opcode);
    }

    chip8->runTimers();

    updateDisplay(chip8->getDisplay(), Chip8::displayWidth,
                  Chip8::displayHeight);
}

void EmuWindow::drawGreeting() {
    std::string greetingText =
        "Drag and drop ROMs here\n\n"
        "             or\n\n"
        "  Press Ctrl + O to open\n\n";

    double fontSize = 20.0f;

    Vector2 textSize =
        MeasureTextEx(GetFontDefault(), greetingText.c_str(), fontSize, 2.0f);

    DrawTextEx(
        GetFontDefault(), greetingText.c_str(),
        Vector2{
            static_cast<float>(GetScreenWidth()) / 2 - (textSize.x / 2),
            (static_cast<float>(GetScreenHeight()) - getMenubarHeight()) / 2 -
                (textSize.y / 2),
        },
        fontSize, 2.0f, BLACK);
}

void EmuWindow::drawFPS() {
    DrawText(TextFormat("fps: %d", GetFPS()), 30, 30, 20, RED);
}

void EmuWindow::displayFFIndicator() { displayIndicator(">> fastforward >>"); }

void EmuWindow::displayPauseIndicator() { displayIndicator("| | paused | |"); }

void EmuWindow::displayIndicator(std::string indicator) {
    float fontSize = 20.0f;
    float spacing = 0.0f;

    float paddingX = 30.0f;
    float paddingY = 16.0f;

    Vector2 textSize =
        MeasureTextEx(GetFontDefault(), indicator.c_str(), fontSize, spacing);

    const float panelWidth = textSize.x + paddingX;
    const float panelHeight = textSize.y + paddingY;

    const float rectangleX =
        (static_cast<float>(GetScreenWidth()) - panelWidth) / 2.0f;
    const float rectangleY =
        getMenubarHeight() +
        (static_cast<float>(GetScreenHeight() - getMenubarHeight() -
                            panelHeight) /
         8.0f);

    DrawRectangleRounded(
        Rectangle{rectangleX, rectangleY, panelWidth, panelHeight}, 0.2f, 10,
        Fade(BLACK, 0.5F));

    DrawTextEx(
        GetFontDefault(), indicator.c_str(),
        Vector2{rectangleX + (paddingX / 2.0f), rectangleY + (paddingY / 2.0f)},
        fontSize, spacing, WHITE);
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

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_P)) {
        std::cout << "Window: Ctrl + P pressed" << std::endl;
        toggleIsPaused();
    }

    // Fastforward: Ctrl + '='
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyDown(KEY_EQUAL))
        inFF = true;
    else
        inFF = false;

    if (IsKeyDown(KEY_ESCAPE) && getInFullscreen()) {
        toggleFullscreen();
    }

    return;
}

void EmuWindow::drawSettingsWindow() {
    const float screenWidth = GetScreenWidth();
    const float screenHeight = GetScreenHeight();

    const float settingsWidth = screenWidth * 2 / 3;
    const float settingsHeight = screenHeight * 2 / 3;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    const float settingsX = (screenWidth / 2 - (settingsWidth / 2));
    const float settingsY = (screenHeight / 2) - (settingsHeight / 2);

    if (GuiWindowBox(
            Rectangle{settingsX, settingsY, settingsWidth, settingsHeight},
            "Settings")) {
        setSettingsOpened(false);
        return;
    }

    const float windowPadding = 10;

    const float fontSize = 30;
    const float fontSpacing = 5;

    Vector2 saveButtonSize =
        MeasureTextEx(GetFontDefault(), "Save", fontSize, fontSpacing);
    Vector2 cancelButtonSize =
        MeasureTextEx(GetFontDefault(), "Cancel", fontSize, fontSpacing);

    // use cancelButton height here for consistency
    if (GuiButton(
            Rectangle{
                settingsX + settingsWidth - windowPadding - saveButtonSize.x,
                settingsY + settingsHeight - windowPadding - saveButtonSize.y,
                saveButtonSize.x, cancelButtonSize.y},
            "Save")) {
        setSettingsOpened(false);
    }
    if (GuiButton(
            Rectangle{
                settingsX + settingsWidth - windowPadding - saveButtonSize.x -
                    windowPadding - cancelButtonSize.x,
                settingsY + settingsHeight - windowPadding - cancelButtonSize.y,
                cancelButtonSize.x, cancelButtonSize.y},
            "Cancel")) {
        setSettingsOpened(false);
    }
}

void EmuWindow::toggleBorderlessWindow() {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    Vector2 position = GetWindowPosition();

    ToggleBorderlessWindowed();

    SetWindowSize(width, height);
    SetWindowPosition(position.x, position.y);
}

void EmuWindow::toggleFullscreen() {
    SetWindowSize(GetMonitorWidth(GetCurrentMonitor()),
                  GetMonitorHeight(GetCurrentMonitor()));
    ToggleFullscreen();
    fullscreenToggle = false;
}

void EmuWindow::updateChip8Scale() {
    SetWindowSize(getChip8Scale() * chip8DisplayWidth,
                  getChip8Scale() * chip8DisplayHeight);
    setScaleUpdated(false);
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
        configPath = std::string(home) + "/.config/PolyEmu.conf";
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
                } else if (var == "rememberWindowSize") {
                    if (val == "1" || val == "true")
                        config.rememberWindowSize = true;
                    else
                        config.rememberWindowSize = false;

                    std::cout << "Loaded config rememberWindowSize: "
                              << config.rememberWindowSize << std::endl;
                } else if (var == "windowWidth") {
                    config.windowWidth = std::stoi(val);
                } else if (var == "windowHeight") {
                    config.windowHeight = std::stoi(val);
                } else if (var == "chip8Scale") {
                    config.chip8Scale = std::stoi(val);

                    std::cout
                        << "Loaded config chip8Scale: " << config.chip8Scale
                        << std::endl;
                } else if (var == "chip8InstPerFrame") {
                    config.chip8InstPerFrame = std::stoi(val);

                    std::cout << "Loaded chip8InstPerFrame: "
                              << config.chip8InstPerFrame << std::endl;

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

    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file [" << configPath
                  << "] :" << std::strerror(errno) << std::endl;

        std::cout << "Creating config file: " << configPath << std::endl;
    }

    file << "showFPS=" << config.showFPS << '\n';
    file << "targetFPS=" << config.targetFPS << '\n';
    file << "rememberWindowSize=" << config.rememberWindowSize << '\n';

    if (config.rememberWindowSize) {
        file << "windowWidth=" << GetScreenWidth() << '\n';
        file << "windowHeight=" << GetScreenHeight() << '\n';
    }

    file << '\n';

    file << "chip8Scale=" << config.chip8Scale << '\n';
    file << "chip8InstPerFrame=" << config.chip8InstPerFrame << '\n';

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

    std::string emulatorFmtString =
        "Emulator;{};{};Increase speed;Decrease speed;Reset "
        "Speed;Settings;Reset";
    const char *fpsText = getShowFPS() ? "Hide FPS" : "Show FPS";
    const char *pauseText = getIsPaused() ? "Resume" : "Pause";

    std::string emulatorDropdown = std::vformat(
        emulatorFmtString, std::make_format_args(fpsText, pauseText));

    if (GuiDropdownBox(Rectangle{60, 0, 85, (float)getMenubarHeight()},
                       emulatorDropdown.c_str(), &menuBar.emulatorActive,
                       menuBar.emulatorEditMode)) {
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
                toggleIsPaused();
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
                setSettingsOpened(!getSettingsOpened());
                break;
            }

            case 7: {
                resetEmu();
                break;
            }
        }
        menuBar.emulatorActive = 0;
    }

    // TODO: expand dropdown when open
    if (GuiDropdownBox(
            Rectangle{60 + 85, 0, 85, (float)getMenubarHeight()},
            "Window;Toggle Fullscreen;Toggle Borderless "
            "Window;Restore Window Size;Increase Scale;Decrease Scale",
            &menuBar.viewActive, menuBar.viewEditMode)) {
        menuBar.viewEditMode = !menuBar.viewEditMode;

        switch (menuBar.viewActive) {
            case 1: {
                fullscreenToggle = true;
                inFullscreen = !inFullscreen;

                break;
            }
            case 2: {
                toggleBorderlessWindow();
                break;
            }
            case 3: {
                config.rememberWindowSize = !config.rememberWindowSize;
                break;
            }
            case 4: {
                int tempScale = getChip8Scale();
                setChip8Scale(tempScale + 1);

                setScaleUpdated(true);
                break;
            }
            case 5: {
                int tempScale = getChip8Scale();
                setChip8Scale(tempScale - 1);

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
