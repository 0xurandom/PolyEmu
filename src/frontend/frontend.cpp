#include "frontend.hpp"

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

#include "../backend/intel8080/intel8080.hpp"
#include "nativefiledialog-extended/src/include/nfd.h"
#include "raygui.h"
#include "raylib.h"

EmuWindow::EmuWindow() {
    initConfig();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth;
    int windowHeight;

    if (config.rememberWindowSize && (config.windowWidth != 0) &&
        (config.windowHeight != 0)) {
        windowWidth = config.windowWidth;
        windowHeight = config.windowHeight;
    } else {
        windowWidth = Chip8::displayWidth * config.chip8Scale;
        windowHeight =
            Chip8::displayHeight * config.chip8Scale + getMenubarHeight();
    }

    InitAudioDevice();
    beepSound = LoadSound("assets/beep.wav");

    si_ufo = LoadSound("assets/ufo_lowpitch.wav");
    si_shoot = LoadSound("assets/shoot.wav");
    si_playerDie = LoadSound("assets/explosion.wav");
    si_invaderDie = LoadSound("assets/invaderkilled.wav");

    si_fleet1 = LoadSound("assets/fastinvader1.wav");
    si_fleet2 = LoadSound("assets/fastinvader2.wav");
    si_fleet3 = LoadSound("assets/fastinvader3.wav");
    si_fleet4 = LoadSound("assets/fastinvader4.wav");
    si_ufoHit = LoadSound("assets/ufo_highpitch.wav");

    InitWindow(windowWidth, windowHeight, this->Header);
    SetExitKey(0);
    SetWindowMinSize(64 * 10, (32 * 10) + getMenubarHeight());
    SetTargetFPS(this->config.targetFPS);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(WHITE));
    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
}

void EmuWindow::closeEmuWindow() {
    UnloadTexture(displayTexture);
    UnloadSound(beepSound);
    CloseAudioDevice();
    CloseWindow();
}

void EmuWindow::initDisplay(int width, int height) {
    Image img = GenImageColor(width, height, BLACK);

    displayTexture = LoadTextureFromImage(img);

    UnloadImage(img);
    SetTextureFilter(displayTexture, TEXTURE_FILTER_POINT);

    pixelBuffer.resize(width * height);
}

void EmuWindow::updateDisplay(const uint8_t* pixels, int width, int height) {}

void EmuWindow::drawDisplay() {
    float availableWidth = static_cast<float>(GetScreenWidth());
    float availableHeight =
        static_cast<float>(GetScreenHeight() - getMenubarHeight());

    DrawRectangle(0, getMenubarHeight(), availableWidth, availableHeight,
                  GetColor(0x181818FF));

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
        getChip8Ptr().setKeyState(i, IsKeyDown(Chip8keymap[i]));
    }
}

std::string EmuWindow::getRomName(const std::string filePath) {
    int lastSlashIndex = filePath.rfind('/');
    int fileExtensionIndex = filePath.rfind('.');

    return filePath.substr(lastSlashIndex + 1,
                           (fileExtensionIndex - lastSlashIndex - 1));
}

void EmuWindow::handleROM(const std::string filePath) {
    const char* extension = GetFileExtension(filePath.c_str());

    if (extension == nullptr) {
        std::cerr << "Error: Unknown file" << std::endl;
        return;
    }

    if (strcmp(extension, ".ch8") == 0) {
        curBackend = Backend::Chip8;
        initDisplay(Chip8::displayWidth, Chip8::displayHeight);

        if (getRomIsLoaded()) {
            getChip8Ptr().reset();
        }

        if (!getChip8Ptr().loadROM(filePath)) {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
            return;
        }

        std::cout << "Chip8: Successfully loaded chip8 rom: " << filePath
                  << std::endl;
        chip8RomPath = filePath;
    } else if (strcmp(extension, ".zip") == 0) {
        if (getRomIsLoaded()) {
            getI8080Ptr().reset();
        }

        if (!getI8080Ptr().loadROM(filePath)) {
            std::cerr << "Error: Couldn't load i8080 rom" << std::endl;
            return;
        }

        curBackend = Backend::i8080;
        initDisplay(i8080::displayWidth, i8080::displayHeight);
    } else if (strcmp(extension, ".p8") == 0) {
        if (getRomIsLoaded()) {
            getPico8Ptr().reset();
        }

        if (!getPico8Ptr().loadROM(filePath)) {
            std::cerr << "Error: Couldn't load chip8 rom" << std::endl;
            return;
        }

        curBackend = Backend::Pico8;
        initDisplay(Pico8::displayWidth, Pico8::displayHeight);
    }

    setRomIsLoaded(true);
    SetWindowTitle(getRomName(filePath).c_str());
}

void EmuWindow::runEmuFrame() {
    if (curBackend == Backend::Chip8) {
        updateChip8KeysPressed();

        int speed = getChip8InstPerFrame();

        if (inFF) speed += chip8FFincrement;

        for (int i = 0; i < speed; i++) {
            Opcode opcode = chip8->getOpcode();
            chip8->handleOpcode(opcode);
        }

        chip8->runTimers();

        if (chip8->getSoundTimer() > 0) {
            if (!IsSoundPlaying(beepSound)) PlaySound(beepSound);
        } else {
            if (IsSoundPlaying(beepSound)) StopSound(beepSound);
        }

        for (int i = 0; i < Chip8::displayWidth * Chip8::displayHeight; i++) {
            if (chip8->getDisplay()[i])
                pixelBuffer[i] = config.chip8Background;
            else
                pixelBuffer[i] = config.chip8Foreground;
        }

        UpdateTexture(displayTexture, pixelBuffer.data());
    } else if (curBackend == Backend::i8080) {
        int speed = i8080HalfinstPerFrame;
        if (inFF) speed += 8000;

        uint8_t port3 = intel8080->getPort3();
        uint8_t port5 = intel8080->getPort5();

        if (port3 != si_lastPort3) {
            if ((port3 & 0x1) && !IsSoundPlaying(si_ufo))
                PlaySound(si_ufo);
            else if (!(port3 & 0x1) && !IsSoundPlaying(si_ufo))
                StopSound(si_ufo);

            if ((port3 & 0x2) && !(si_lastPort3 & 0x2)) PlaySound(si_shoot);
            if ((port3 & 0x4) && !(si_lastPort3 & 0x4)) PlaySound(si_playerDie);
            if ((port3 & 0x8) && !(si_lastPort3 & 0x8))
                PlaySound(si_invaderDie);

            si_lastPort3 = port3;
        }

        if (port5 != si_lastPort5) {
            if ((port5 & 0x1) && !(si_lastPort5 & 0x1)) PlaySound(si_fleet1);
            if ((port5 & 0x2) && !(si_lastPort5 & 0x2)) PlaySound(si_fleet2);
            if ((port5 & 0x4) && !(si_lastPort5 & 0x4)) PlaySound(si_fleet3);
            if ((port5 & 0x8) && !(si_lastPort5 & 0x8)) PlaySound(si_fleet4);
            if ((port5 & 0x10) && !(si_lastPort5 & 0x10)) PlaySound(si_ufoHit);
        }

        for (int i = 0; i < speed; i++) {
            getI8080Ptr().emulate8080();
        }
        getI8080Ptr().generateInterrupt(1);
        getI8080Ptr().updateKeys();
        for (int i = 0; i < speed; i++) {
            getI8080Ptr().emulate8080();
        }
        getI8080Ptr().generateInterrupt(2);

        getI8080Ptr().renderScreen();

        const uint32_t* screenBuffer = getI8080Ptr().getScreenBuffer();

        for (int i = 0; i < i8080::displayWidth * i8080::displayHeight; i++) {
            pixelBuffer[i] = GetColor(screenBuffer[i]);
        }

        UpdateTexture(displayTexture, pixelBuffer.data());
    } else if (curBackend == Backend::Pico8) {
        getPico8Ptr().renderScreen(pixelBuffer);
        UpdateTexture(displayTexture, pixelBuffer.data());
    }
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

    nfdu8char_t* outPath;
    nfdu8filteritem_t filters[3] = {{"Chip8 ROMs", "ch8"},
                                    {"Space invaders zips", "zip"},
                                    {"Pico8 ROMs", "p8"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 3;

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

    // Pause: Ctrl + P
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

    // Toggle settings: Ctrl + ','
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_COMMA)) {
        std::cout << "Window: Ctrl + , Pressed" << std::endl;
        setSettingsOpened(!getSettingsOpened());
    }

    // Esc to exit fullscreen
    if (IsKeyPressed(KEY_ESCAPE) && getInFullscreen()) {
        inFullscreen = false;
        toggleFullscreen();
    }

    // Toggle fullscreen: Ctrl + F
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_F)) {
        std::cout << "Window: Ctrl + F pressed" << std::endl;

        inFullscreen = !inFullscreen;
        toggleFullscreen();
    }

    // F5: Save State
    if (IsKeyPressed(KEY_F5)) {
        if (curBackend == Backend::Chip8) {
            std::string savePath = gettChip8RomPath() + "_5.save";
            getChip8Ptr().saveState(savePath);
        } else if (curBackend == Backend::i8080) {
        }
    }

    // F7: Load State
    if (IsKeyPressed(KEY_F7)) {
        if (curBackend == Backend::Chip8) {
            std::string savePath = gettChip8RomPath() + "_5.save";
            if (std::filesystem::exists(savePath)) {
                getChip8Ptr().loadState(savePath);
            }
        }
    }

    return;
}

void EmuWindow::drawSettingsWindow() {
    const float screenWidth = GetScreenWidth();
    const float screenHeight = GetScreenHeight();

    const float settingsWidth = screenWidth * 0.5;
    const float settingsHeight = screenHeight * 0.9f;

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

    Rectangle scrollPanelBounds = {
        settingsX + windowPadding,
        settingsY + 35.0f,
        settingsWidth - (windowPadding * 2),
        settingsHeight - 45.0f,
    };

    const int itemCount = 16;
    const float rowHeight = 40.0f;
    const float topPadding = 10.0f;
    const float bottomPadding = 20.0f;

    const float totalHeight =
        topPadding + (rowHeight * itemCount) + bottomPadding;

    Rectangle scrollPanelContent = {0, 0, scrollPanelBounds.width - 20.0f,
                                    totalHeight};

    GuiScrollPanel(scrollPanelBounds, nullptr, scrollPanelContent,
                   &settingsScrollPos, &settingsScrollView);

    const float itemStartX = scrollPanelBounds.x + 10.0f;
    const float itemStartY = scrollPanelBounds.y + 10.0f;

    BeginScissorMode(settingsScrollView.x, settingsScrollView.y,
                     settingsScrollView.width, settingsScrollView.height);

    for (int i = 0; i < 16; i++) {
        float currentY = itemStartY + (i * 40) + settingsScrollPos.y;

        GuiLabel(Rectangle{itemStartX, currentY, 100.0f, 30.0f},
                 TextFormat("Key %c", chip8Buttons[i]));
        if (GuiButton(Rectangle{itemStartX + 120.0f, currentY, 150.0f, 30.0f},
                      "Bind Key")) {
        }
    }

    float backgroundY = itemStartY + (0 * 40) + settingsScrollPos.y;
    GuiLabel(Rectangle{itemStartX, backgroundY, 100.0f, 30.0f},
             "Chip8 Background: ");
    if (GuiDropdownBox(
            Rectangle{itemStartX + 120.0f, backgroundY, 150.0f, 30.0f},
            "White;Black;Red;Maroon;Green;Dark Green;Blue;Dark "
            "Blue;Purple;DarkPurple",
            &settingsMenu.backgroundActive, settingsMenu.backgroundEditMode)) {
        settingsMenu.backgroundEditMode = !settingsMenu.backgroundEditMode;
    }

    float foregroundY = itemStartY + (1 * 40) + settingsScrollPos.y;
    GuiLabel(Rectangle{itemStartX, backgroundY, 100.0f, 30.0f},
             "Chip8 Foreground:");
    if (GuiDropdownBox(
            Rectangle{itemStartX + 120.0f, foregroundY, 150.0f, 30.0f},
            "White;Black;Red;Maroon;Green;Dark Green;Blue;Dark "
            "Blue;Purple;DarkPurple",
            &settingsMenu.foregroundActive, settingsMenu.foregroundEditMode)) {
        settingsMenu.foregroundEditMode = !settingsMenu.foregroundEditMode;
    }

    EndScissorMode();
}

void EmuWindow::drawHelpWindow() {
    const float screenWidth = GetScreenWidth();
    const float screenHeight = GetScreenHeight();

    const float helpWidth = screenWidth / 3;
    const float helpHeight = screenHeight * 0.9f;

    const float helpX = (screenWidth / 2 - (helpWidth / 2));
    const float helpY = (screenHeight / 2) - (helpHeight / 2);

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

    if (GuiWindowBox(Rectangle{helpX, helpY, helpWidth, helpHeight}, "Help")) {
        setHelpOpened(false);
        return;
    }

    const char* shortcuts[] = {"Ctrl + O : Open Rom",
                               "Ctrl + R : Reset Emulator",
                               "Ctrl + P : Pause/Resume",
                               "Ctrl + F : Toggle Fullscreen",
                               "Esc      : Exit Fullscreen",
                               "Ctrl + , : Open Settings",
                               "Ctrl + = : Hold to fast forward",
                               "F5       : Save State (Slot 5)",
                               "F7       : Load State (SLot 5)"};

    int shortcutCount = sizeof(shortcuts) / sizeof(shortcuts[0]);

    float currentY = helpY + 40.0f;

    for (int i = 0; i < shortcutCount; i++) {
        GuiLabel(Rectangle{helpX + 20, currentY, helpWidth - 40, 30},
                 shortcuts[i]);
        currentY += 20;
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
    if (inFullscreen) {
        prevWindowState.width = GetScreenWidth();
        prevWindowState.height = GetScreenHeight();
        prevWindowState.position = GetWindowPosition();

        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(prevWindowState.width, prevWindowState.height);
        SetWindowPosition(prevWindowState.position.x,
                          prevWindowState.position.y);
    }

    fullscreenToggle = false;
}

void EmuWindow::updateScale() {
    if (curBackend == Backend::Chip8) {
        SetWindowSize(
            getChip8Scale() * Chip8::displayWidth,
            (getChip8Scale() * Chip8::displayHeight) + getMenubarHeight());
    } else if (curBackend == Backend::i8080) {
        SetWindowSize(
            config.i8080Scale * i8080::displayWidth,
            (config.i8080Scale * i8080::displayHeight) + getMenubarHeight());
    }
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
    char* home = std::getenv("HOME");
    if (home != NULL)
        configPath = std::string(home) + "/.config/Polyemu/PolyEmu.conf";
    else
        std::cerr << "Error: Could not get home path for linux" << std::endl;

#elif defined(_WIN32)
    char* appdata = std::getenv("APPDATA");

    if (appdata != NULL)
        configPath = std::string(appdata) + "\\PolyEmu\PolyEmu.conf";
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
    std::filesystem::path pathObj(configPath);
    if (pathObj.has_parent_path() &&
        !std::filesystem::exists(pathObj.parent_path())) {
        std::filesystem::create_directories(pathObj.parent_path());
    }

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

    const float fileWidth = 70.0f;
    const float emulatorWidth = 125.0f;
    const float windowWidth = 150.0f;
    const float helpWidth = 100.0f;

    const float fileX = 0.0f;
    const float emulatorX = fileX + fileWidth;
    const float windowX = emulatorX + emulatorWidth;
    const float helpX = windowX + windowWidth;

    if (GuiDropdownBox(
            Rectangle{fileX, 0, fileWidth, (float)getMenubarHeight()},
            "File;Open;Exit", &menuBar.fileActive, menuBar.fileEditMode)) {
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

    std::string emulatorFmtString = "Emulator;{};{}";

    if (curBackend == Backend::Chip8) {
        emulatorFmtString +=
            ";Increase speed;Decrease speed;Reset Speed;Save/Load "
            "States;Reset";
    } else if (curBackend == Backend::i8080) {
        emulatorFmtString +=
            ";Increase speed;Decrease speed;Reset Speed;Save/Load "
            "States;Reset";
    } else if (curBackend == Backend::Pico8) {
        emulatorFmtString += ";Reset";
    }

    const char* fpsText = getShowFPS() ? "Hide FPS" : "Show FPS";
    const char* pauseText = getIsPaused() ? "Resume" : "Pause";

    std::string emulatorDropdown = std::vformat(
        emulatorFmtString, std::make_format_args(fpsText, pauseText));

    float emulatorDropdownWidth = menuBar.emulatorEditMode ? 250.0f : 85.0f;

    if (GuiDropdownBox(
            Rectangle{emulatorX, 0, emulatorWidth, (float)getMenubarHeight()},
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
            case 2: toggleIsPaused(); break;

            default: {
                if (curBackend == Backend::Pico8) {
                    if (menuBar.emulatorActive == 3) resetEmu();
                } else {
                    if (menuBar.emulatorActive == 3) {
                        if (curBackend == Backend::Chip8) {
                            int speed = getChip8InstPerFrame();
                            setchip8InstPerFrame(speed + 8);
                        } else if (curBackend == Backend::i8080) {
                            i8080HalfinstPerFrame += 1000;
                        }
                    } else if (menuBar.emulatorActive == 4) {
                        if (curBackend == Backend::Chip8) {
                            int speed = getChip8InstPerFrame();
                            setchip8InstPerFrame(speed - 8);
                        } else if (curBackend == Backend::i8080) {
                            i8080HalfinstPerFrame -= 1000;
                        }
                    } else if (menuBar.emulatorActive == 5) {
                        if (curBackend == Backend::Chip8)
                            resetchip8InstPerFrame();
                        else if (curBackend == Backend::i8080)
                            i8080HalfinstPerFrame = 4000;
                        break;
                    } else if (menuBar.emulatorActive == 6) {
                        setStatesOpened(!getStatesOpened());
                    } else if (menuBar.emulatorActive == 7) {
                        resetEmu();
                    }
                }
            }
        }
        menuBar.emulatorActive = 0;
    }

    float windowDropdownWidth = menuBar.fileEditMode ? 250.0f : 85.0f;

    if (GuiDropdownBox(
            Rectangle{windowX, 0, windowWidth, (float)getMenubarHeight()},
            "Window;Toggle Fullscreen;Toggle Borderless "
            "Window;Remember Window Size;Increase Scale;Decrease Scale",
            &menuBar.viewActive, menuBar.viewEditMode)) {
        menuBar.viewEditMode = !menuBar.viewEditMode;

        switch (menuBar.viewActive) {
            case 1: {
                fullscreenToggle = true;
                inFullscreen = !inFullscreen;

                break;
            }
            case 2: toggleBorderlessWindow(); break;

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

    if (GuiDropdownBox(
            Rectangle{helpX, 0, helpWidth, (float)getMenubarHeight()},
            "Help;Show Shortcuts", &menuBar.helpActive, menuBar.helpEditMode)) {
        menuBar.helpEditMode = !menuBar.helpEditMode;
        switch (menuBar.helpActive) {
            case 1: {
                setHelpOpened(true);
                break;
            }
        }
        menuBar.helpActive = 0;
    }
}

void EmuWindow::drawStatesWindow() {
    const float screenWidth = GetScreenWidth();
    const float screenHeight = GetScreenHeight();

    const float statesWidth = screenWidth * 0.43f;
    const float statesHeight = screenHeight * 0.9f;

    const float statesX = (screenWidth / 2 - (statesWidth / 2));
    const float statesY = (screenHeight / 2) - (statesHeight / 2);

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5));

    if (!getRomIsLoaded()) {
        if (GuiMessageBox(
                Rectangle{statesX, statesY, screenWidth / 3, screenHeight / 3},
                "Save States", "Load a ROM first", nullptr)) {
            return;
        }
    }

    if (GuiWindowBox(Rectangle{statesX, statesY, statesWidth, statesHeight},
                     "Save States")) {
        setStatesOpened(false);
        return;
    }

    float currentY = statesY + 40.0f;

    for (int i = 1; i < 6; i++) {
        GuiLabel(Rectangle{statesX + 20, currentY, 60, 30},
                 TextFormat("Slot %d", i));
        std::string savePath = "";

        if (curBackend == Backend::Chip8) {
            savePath = gettChip8RomPath() + "_" + std::to_string(i) + ".save";
        } else if (curBackend == Backend::i8080) {
        }

        if (GuiButton(Rectangle{statesX + 80, currentY, 80, 30}, "Save")) {
            bool successfulSave = false;

            if (curBackend == Backend::Chip8) {
                successfulSave = getChip8Ptr().saveState(savePath);
            } else if (curBackend == Backend::i8080) {
                successfulSave = false;
            }

            if (!successfulSave)
                std::cerr << "Error: Could not save state to " << savePath
                          << std::endl;
        }

        bool stateExists = std::filesystem::exists(savePath);
        if (!stateExists) GuiDisable();

        if (GuiButton(Rectangle{statesX + 170, currentY, 80, 30}, "Load")) {
            bool successfulLoad = false;

            if (curBackend == Backend::Chip8) {
                savePath = successfulLoad = getChip8Ptr().loadState(savePath);
            } else if (curBackend == Backend::i8080) {
                successfulLoad = false;
            }

            if (!successfulLoad)
                std::cerr << "Error: Could not load state from " << savePath
                          << std::endl;
        }

        if (!stateExists) GuiEnable();
        currentY += 40.0f;
    }
}

void EmuWindow::resetEmu() {
    getChip8Ptr().reset();
    getChip8Ptr().loadROM(gettChip8RomPath());
}
