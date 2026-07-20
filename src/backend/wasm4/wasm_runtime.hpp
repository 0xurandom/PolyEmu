#pragma once

#include <cstdint>
#include <vector>

enum WasmValueKind {
    Int32,
    Int64,
    Float32,
    Float64,
};

struct WasmValue {
    WasmValueKind kind;

    union {
        uint32_t i32;
        uint64_t i64;
        float f32;
        float f64;
    };
};

class WasmRuntime {
   public:
   private:
    std::vector<WasmValue> stack;

    std::vector<uint8_t> ram;

    void handleOpcode(uint8_t code);
};
