#pragma once

#include <cstdint>

// Desktop-Kompatibilitätsschicht:
// Stellt zentrale 3DS-Typen und Tasten-Flags für den Win32-Simulator bereit.
using u32 = std::uint32_t;
using u64 = std::uint64_t;

struct touchPosition {
    // Touch-Koordinaten (pixelbasiert, analog zur 3DS-API).
    std::uint16_t px = 0;
    std::uint16_t py = 0;
};

constexpr u32 KEY_A = 1u << 0;
constexpr u32 KEY_B = 1u << 1;
constexpr u32 KEY_SELECT = 1u << 2;
constexpr u32 KEY_START = 1u << 3;
constexpr u32 KEY_DRIGHT = 1u << 4;
constexpr u32 KEY_DLEFT = 1u << 5;
constexpr u32 KEY_DUP = 1u << 6;
constexpr u32 KEY_DDOWN = 1u << 7;
constexpr u32 KEY_R = 1u << 8;
constexpr u32 KEY_L = 1u << 9;
constexpr u32 KEY_X = 1u << 10;
constexpr u32 KEY_Y = 1u << 11;
constexpr u32 KEY_TOUCH = 1u << 20;
constexpr u32 KEY_CPAD_RIGHT = 1u << 24;
constexpr u32 KEY_CPAD_LEFT = 1u << 25;
constexpr u32 KEY_CPAD_UP = 1u << 26;
constexpr u32 KEY_CPAD_DOWN = 1u << 27;

constexpr u32 KEY_RIGHT = KEY_DRIGHT;
constexpr u32 KEY_LEFT = KEY_DLEFT;
constexpr u32 KEY_UP = KEY_DUP;
constexpr u32 KEY_DOWN = KEY_DDOWN;
