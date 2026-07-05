#include <fstream>
#include <iostream>
#include <string>

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
