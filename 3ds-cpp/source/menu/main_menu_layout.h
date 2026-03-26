#pragma once

namespace mainmenu {
namespace layout {

struct Rect {
    int x;
    int y;
    int w;
    int h;
};

inline bool contains(const Rect& rect, int px, int py) {
    return px >= rect.x && px < rect.x + rect.w && py >= rect.y && py < rect.y + rect.h;
}

inline Rect homeButtonRect(int index) {
    return {50, 48 + index * 48, 220, 32};
}

inline Rect playSlotLeftRect() {
    return {44, 8, 40, 28};
}

inline Rect playSlotRightRect() {
    return {236, 8, 40, 28};
}

inline Rect playButtonRect(int index) {
    return {50, 48 + index * 48, 220, 32};
}

inline Rect optionsButtonRect(int index) {
    return {40, 42 + index * 48, 240, 32};
}

inline Rect manualTopicRect(int index) {
    return {20, 40 + index * 36, 280, 32};
}

inline Rect manualBackRect() {
    return {20, 220, 280, 18};
}

inline Rect manualPageBackRect() {
    return {20, 192, 280, 20};
}

inline Rect manualPageScrollUpRect() {
    return {0, 0, 320, 48};
}

inline Rect manualPageScrollDownRect() {
    return {0, 192, 320, 48};
}

} // namespace layout
} // namespace mainmenu
