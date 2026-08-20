#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

#include "api.hpp"
using namespace std;

// bool p_cartdata(string id) {}

filesystem::path getSaveDir() {
#ifdef __linux__
    if (const char* xdg = getenv("XDG_CONFIG_HOME")) {
        return filesystem::path(xdg) / "polyemu";
    }

    const char* home = std::getenv("HOME");
    return filesystem::path(home) / ".config" / "polyemu";
#endif
}

bool saveDirExists() {
    filesystem::path dir = getSaveDir();

    error_code ec;
    if (!filesystem::exists(dir)) {
        if (!filesystem::create_directories(dir, ec)) {
            cerr << "Error: Could not create save directory: " << ec.message()
                 << endl;
            return false;
        }
    }
    return true;
}

bool writeCartData(const string& id, uint8_t index, uint8_t data) {
    if (!saveDirExists() || index > 64) return false;

    filesystem::path filePath = getSaveDir() / (id + ".txt");

    ofstream out(filePath, ios::in | ios::out);

    if (!out.is_open()) {
        cerr << "Error: File count not be opened\n";
        return false;
    }

    uintmax_t size = filesystem::file_size(filePath);
    if (size != 64) {
        filesystem::resize_file(filePath, 64);
    }

    out.seekp(index, ios::beg);
    out << data;
    out.close();
    return true;
}

uint8_t readCartData(const string& id, uint8_t index) {
    if (!saveDirExists() || index > 64) return false;

    filesystem::path filePath = getSaveDir() / (id + ".txt");
    if (!filesystem::exists(filePath)) {
        cerr << "Warning: Attempted to read cartdata which does not exist for "
                "id: "
             << id << endl;
        return 0;
    }

    ifstream in(filePath, ios::binary);

    if (!in) {
        cerr << "Error: file could not be opened: " << filePath << endl;
        return 0;
    }

    uint8_t data;
    in.seekg(index, ios::beg);
}
