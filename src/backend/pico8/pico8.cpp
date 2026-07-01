#include "pico8.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum CartSection {
    LUA,
    GFX,
    MAP,
};

bool loadCartridge(const string &filepath);

int main() {
    string path = "testcartridge.p8";

    return loadCartridge(path);
}

bool loadCartridge(const string &filepath) {
    ifstream file(filepath);

    if (!file.is_open()) {
        cout << "Could not open file\n";
        return false;
    }

    string line;
    // TODO: convert raw lua into
    // regular lua
    string rawLua = "";
    CartSection cartSection;

    while (getline(file, line)) {
        // TODO: use switch here

        if (line == "__lua__") {
            cartSection = LUA;
            continue;

        } else if (line == "__gfx__") {
            cartSection = GFX;
            continue;

        } else if (line == "__map__") {
            cartSection = MAP;
            continue;
        }

        switch (cartSection) {
            case LUA: {
                rawLua += line + '\n';
                break;
            }

            case GFX: {
                // TODO
                break;
            }

            case MAP: {
                // TODO
                break;
            }
        }
    }

    file.close();
    cout << "Successfully loaded file\n";
    cout << "Lua:" << rawLua << '\n';
    return true;
}

vector<Token> tokeniseLua(string line) {
    vector<Token> tokenArr;

    for (char c : line) {
        Token token;

        switch (c) {
            case '+': {
                token.kind = PLUS;
                break;
            }

            case '-': {
                token.kind = MINUS;
                break;
            }

            case '*': {
                token.kind = ASTERISK;
                break;
            }

            case '%': {
                token.kind = PERCENT;
                break;
            }

            case '(': {
                token.kind = LPAREN;
                break;
            }

            case ')': {
                token.kind = RPAREN;
                break;
            }

            case '=': {
                token.kind = EQUALS;
                break;
            }

            case '<': {
                token.kind = LESS_THAN;
                break;
            }

            case '>': {
                token.kind = GREATER_THAN;
                break;
            }

            default: {
                cout << "Unknown token:" << c << '\n';
                break;
            }
        }
        tokenArr.push_back(token);
    }
}

string processLua(vector<Token> tokenArr) {}
