#include "chip8.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>

Opcode Chip8::getOpcode() {
    Opcode opcode(static_cast<uint16_t>((ram[pc] << 8) |
                                        ((pc < 4094) ? (ram[pc + 1]) : 0)));

    pc += 2;

    return opcode;
}

bool Chip8::loadROM(const std::string &filepath) {
    const unsigned int startAddr = 0x200;
    const std::streamsize maxSize = sizeof(ram) - startAddr;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open chip8 rom file" << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > maxSize) {
        std::cerr << "Error: ROM size is larger than can be allocated in ram"
                  << std::endl;
        return false;
    }

    if (!file.read(reinterpret_cast<char *>(&ram[startAddr]), size)) {
        std::cerr << "Error: Could not read chip8 rom" << std::endl;
        return false;
    } else {
        return true;
    }
}

void Chip8::handleOpcode(Opcode opcode) {
    uint16_t firstNibble = (opcode.code & 0xF000) >> 12;

    switch (firstNibble) {
        case 0x0: {
            switch (opcode.NN) {
                // 00E0
                case 0xE0:
                    clearScreen();
                    break;

                // 00EE
                case 0xEE: {
                    returnFromSubroutine();
                    break;
                }

                default: {
                    std::cerr << "Error: Unknown opcode starting with 0x0: "
                              << opcode.NN << std::endl;
                    break;
                }
            }

            break;
        }

        // 1NNN
        case 0x1: {
            jump(opcode);
            break;
        }

        // 2NNN
        case 0x2: {
            callSubroutine(opcode);
            break;
        }

        // 3XNN
        case 0x3: {
            if (V[opcode.X] == opcode.NN) skipInstruction();
            break;
        }

        // 4XNN
        case 0x4: {
            if (V[opcode.X] != opcode.NN) skipInstruction();
            break;
        }

        // 5XY0
        case 0x5: {
            if (V[opcode.X] == V[opcode.Y]) skipInstruction();
            break;
        }

        // 9XY0
        case 0x9: {
            if (V[opcode.X] != V[opcode.Y]) skipInstruction();
            break;
        }

        // 6XNN
        case 0x6: {
            setVarRegister(opcode);
            break;
        }

        // 7XNN
        case 0x7: {
            addValToRegister(opcode);
            break;
        }

        case 0x8: {
            switch (opcode.N) {
                case 0x0: {
                    // 8XY0
                    setVXtoVY(opcode);
                    break;
                }

                case 0x1: {
                    // 8XY1
                    binaryOr(opcode);
                    break;
                }

                case 0x2: {
                    // 8XY2
                    binaryAnd(opcode);
                    break;
                }

                case 0x3: {
                    // 8XY3
                    binaryXor(opcode);
                    break;
                }

                case 0x4: {
                    // 8XY4
                    addValToRegisterOverflow(opcode);

                    break;
                }

                case 0x5: {
                    // 8XY5
                    bool underflow;
                    subtractYfromX(opcode, &underflow);

                    V[15] = underflow;

                    break;
                }

                case 0x7: {
                    // 8XY7
                    bool underflow;
                    subtractXfromY(opcode, &underflow);

                    V[15] = underflow;

                    break;
                }

                case 0x6: {
                    // 8XY6
                    uint8_t shiftedBit;
                    shiftRight(opcode, &shiftedBit);

                    V[15] = shiftedBit;
                    break;
                }

                case 0xE: {
                    // 8XYE

                    uint8_t shiftedBit;
                    shiftLeft(opcode, &shiftedBit);

                    V[15] = shiftedBit;
                    break;
                }
            }

            break;
        }

        // ANNN
        case 0xA: {
            setIndex(opcode);
            break;
        }

        // BNNN
        case 0xB: {
            // TODO: add configurable BXNN
            jumpWithOffset(opcode);
            break;
        }

        // CXNN
        case 0xC: {
            randomNum(opcode);
            break;
        }

        // DXYN
        case 0xD: {
            draw(opcode);
            break;
        }

        case 0xE: {
            switch (opcode.NN) {
                // EX9E
                case 0x9E: {
                    skipIfKey(opcode);
                    break;
                }

                // EXA1
                case 0xA1: {
                    skipIfNotKey(opcode);
                    break;
                }
            }

            break;
        }

        case 0xF: {
            switch (opcode.NN) {
                // FX07
                case 0x07: {
                    setVXToDelay(opcode);
                    break;
                }

                // FX15
                case 0x15: {
                    setDelayToVX(opcode);
                    break;
                }

                // FX18
                case 0x18: {
                    setSoundToVX(opcode);
                    break;
                }

                // FX1E
                case 0x1E: {
                    bool overflow;
                    addToIndex(opcode, &overflow);

                    V[15] = overflow;

                    break;
                }

                // FX0A
                case 0x0A: {
                    getKey(opcode);
                    break;
                }

                // FX29
                case 0x29: {
                    setFont(opcode);
                    break;
                }

                // FX33
                case 0x33: {
                    binaryDecimalConversion(opcode);
                    break;
                }

                // FX55
                case 0x55: {
                    storeMem(opcode);
                    break;
                }

                // FX65
                case 0x65: {
                    loadMem(opcode);
                    break;
                }
            }

            break;
        }
    }
}

void Chip8::draw(Opcode opcode) {
    uint8_t VX = V[opcode.X];
    uint8_t VY = V[opcode.Y];

    uint8_t startX = VX % displayWidth;
    uint8_t startY = VY % displayHeight;

    V[15] = 0;

    for (size_t row = 0; row < opcode.N; row++) {
        uint8_t Y = startY + row;
        if (Y >= displayHeight) break;

        uint8_t spriteByte = ram[I + row];

        for (size_t col = 0; col < 8; col++) {
            uint8_t X = startX + col;
            if (X >= displayWidth) break;

            uint8_t spriteBit = (spriteByte >> (7 - col)) & 0x1;
            uint8_t screenBit = display[(Y * displayWidth) + X];

            if ((spriteBit == 1) && (screenBit == 1)) {
                display[(Y * displayWidth) + X] = 0;
                V[15] = 1;

            } else if ((spriteBit == 1) && (screenBit == 0)) {
                display[(Y * displayWidth) + X] = 1;
                // V[15] = 0;
            }
        }
    }
}

void Chip8::reset() {
    clearScreen();

    memset(ram, 0, sizeof(ram));
    memset(keys, 0, sizeof(keys));
    memset(stack, 0, sizeof(stack));
    memset(V, 0, sizeof(V));

    loadFonts();

    I = 0;
    sp = 0;
    pc = 0x200;
    delay_timer = 0;
    sound_timer = 0;
}

void Chip8::runTimers() {
    if (delay_timer > 0) delay_timer--;

    if (sound_timer > 0) sound_timer--;
}

void Chip8::clearScreen() { std::memset(display, 0, sizeof(display)); }

void Chip8::callSubroutine(Opcode opcode) {
    stack[sp] = pc;
    sp++;

    pc = opcode.NNN;
}

void Chip8::returnFromSubroutine() {
    sp--;
    pc = stack[sp];
}

void Chip8::setVarRegister(Opcode opcode) {
    if (opcode.X < 0 || opcode.X > 15) {
        std::cerr << "Error: setVarRegister received invalid register num: "
                  << (int)opcode.X << std::endl;
    }

    V[opcode.X] = opcode.NN;
}

void Chip8::addValToRegister(Opcode opcode) {
    // TODO: move this error after fetching opcode
    if (opcode.X < 0 || opcode.X > 15) {
        std::cerr << "Error: addValToRegister received invalid register num: "
                  << (int)opcode.X << std::endl;
    }

    int result = V[opcode.X] + opcode.NN;

    V[opcode.X] += opcode.NN;
}

void Chip8::addValToRegisterOverflow(Opcode opcode) {
    int result = V[opcode.X] + V[opcode.Y];

    if (result > 255)
        V[15] = 1;
    else
        V[15] = 0;

    V[opcode.X] += V[opcode.Y];
}

void Chip8::setVXtoVY(Opcode opcode) { V[opcode.X] = V[opcode.Y]; }

void Chip8::setIndex(Opcode opcode) { I = opcode.NNN; }

void Chip8::binaryOr(Opcode opcode) { V[opcode.X] = V[opcode.X] | V[opcode.Y]; }

void Chip8::binaryAnd(Opcode opcode) {
    V[opcode.X] = V[opcode.X] & V[opcode.Y];
}

void Chip8::binaryXor(Opcode opcode) {
    V[opcode.X] = V[opcode.X] ^ V[opcode.Y];
}

void Chip8::subtractYfromX(Opcode opcode, bool *underflow) {
    if (V[opcode.X] >= V[opcode.Y])
        *underflow = true;
    else
        *underflow = false;

    V[opcode.X] = V[opcode.X] - V[opcode.Y];
}

void Chip8::subtractXfromY(Opcode opcode, bool *underflow) {
    if (V[opcode.Y] >= V[opcode.X])
        *underflow = true;
    else
        *underflow = false;

    V[opcode.X] = V[opcode.Y] - V[opcode.X];
}

void Chip8::shiftRight(Opcode opcode, uint8_t *shiftedBit) {
    // TODO: Maybe make this assignment configurable
    V[opcode.X] = V[opcode.Y];

    *shiftedBit = V[opcode.X] & 0x1;

    V[opcode.X] >>= 1;
}

void Chip8::shiftLeft(Opcode opcode, uint8_t *shiftedBit) {
    // TODO: Maybe make this assignment configurable
    V[opcode.X] = V[opcode.Y];

    *shiftedBit = (V[opcode.X] & 0x80) >> 7;

    V[opcode.X] <<= 1;
}

void Chip8::skipInstruction() { pc += 2; }

void Chip8::jump(Opcode opcode) { pc = opcode.NNN; }

void Chip8::jumpWithOffset(Opcode opcode) { pc = opcode.NNN + V[0]; }

void Chip8::randomNum(Opcode opcode) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> distribution(0, 255);

    uint8_t randomNum = distribution(gen);

    randomNum &= opcode.NN;

    V[opcode.X] = randomNum;
}

void Chip8::skipIfKey(Opcode opcode) {
    if (keys[V[opcode.X]] == true) pc += 2;
}

void Chip8::skipIfNotKey(Opcode opcode) {
    if (keys[V[opcode.X]] == false) pc += 2;
}

void Chip8::setVXToDelay(Opcode opcode) { V[opcode.X] = delay_timer; }

void Chip8::setDelayToVX(Opcode opcode) { delay_timer = V[opcode.X]; }

void Chip8::setSoundToVX(Opcode opcode) { sound_timer = V[opcode.X]; }

void Chip8::addToIndex(Opcode opcode, bool *overflow) {
    uint16_t sum = I + V[opcode.X];

    if (sum > 0x0FFF)
        *overflow = true;
    else
        *overflow = false;

    I = sum;
}

void Chip8::getKey(Opcode opcode) {
    bool keyPressed = false;

    for (int i = 0; i < 16; i++) {
        if (keys[i]) {
            V[opcode.X] = i;
            keyPressed = true;
        }
    }

    if (!keyPressed) pc -= 2;

    return;
}

void Chip8::setFont(Opcode opcode) {
    uint8_t num = V[opcode.X];

    I = 0x50 + (num * 5);
}

void Chip8::binaryDecimalConversion(Opcode opcode) {
    uint8_t num = V[opcode.X];

    ram[I] = num / 100;
    ram[I + 1] = (num / 10) % 10;
    ram[I + 2] = num % 10;
}

void Chip8::storeMem(Opcode opcode) {
    for (uint8_t i = 0; i <= opcode.X; i++) {
        ram[I + i] = V[i];
    }
    // TODO: add this as a toggle
    I += opcode.X + 1;
}

void Chip8::loadMem(Opcode opcode) {
    for (uint8_t i = 0; i <= opcode.X; i++) {
        V[i] = ram[I + i];
    }
    I += opcode.X + 1;
}

bool Chip8::isChip8KeyPressed() {
    for (int i = 0; i < 16; i++) {
        if (keys[i] == true) return true;
    }
    return false;
}

void Chip8::loadFonts() {
    std::copy(std::begin(fonts), std::end(fonts), ram + 0x50);
}
