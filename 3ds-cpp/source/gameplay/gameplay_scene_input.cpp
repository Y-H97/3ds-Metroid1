#include "gameplay_scene.h"

void GameplayScene::setControlsSwapped(bool swapped) {
    controlsSwapped = swapped;
}

void GameplayScene::setShowFpsEnabled(bool enabled) {
    showFpsEnabled = enabled;
}

bool GameplayScene::getShowFpsEnabled() const {
    return showFpsEnabled;
}

void GameplayScene::handleInput(u32 kDown, u32 kHeld) {
    (void)kHeld;
    jumpPressed = false;

    const u32 topLeft = controlsSwapped ? KEY_DLEFT : KEY_CPAD_LEFT;
    const u32 topRight = controlsSwapped ? KEY_DRIGHT : KEY_CPAD_RIGHT;
    const u32 topJump = KEY_A;

    const u32 bottomLeft = controlsSwapped ? KEY_CPAD_LEFT : KEY_DLEFT;
    const u32 bottomRight = controlsSwapped ? KEY_CPAD_RIGHT : KEY_DRIGHT;
    const u32 bottomUp = controlsSwapped ? KEY_CPAD_UP : KEY_DUP;
    const u32 bottomDown = controlsSwapped ? KEY_CPAD_DOWN : KEY_DDOWN;

    moveLeftHeld = (kHeld & topLeft) != 0;
    moveRightHeld = (kHeld & topRight) != 0;
    jumpHeld = (kHeld & topJump) != 0;
    jumpPressed = (kDown & topJump) != 0;

    if (kDown & KEY_SELECT) {
        requestMenu = true;
        return;
    }

    if (kDown & bottomLeft) bottomMode = (bottomMode + 3) % 4;
    if (kDown & bottomRight) bottomMode = (bottomMode + 1) % 4;

    if (kDown & KEY_TOUCH) {
        touchPosition tp;
        hidTouchRead(&tp);
        if (tp.py >= 220) {
            bottomMode = tp.px / 80;
            if (bottomMode < 0) bottomMode = 0;
            if (bottomMode > 3) bottomMode = 3;
        } else if (bottomMode == TAB_SETTINGS) {
            if (tp.py >= 54 && tp.py <= 96) {
                settingsSelection = 0;
                showFpsEnabled = !showFpsEnabled;
            } else if (tp.py >= 104 && tp.py <= 146) {
                settingsSelection = 1;
                requestMenu = true;
            }
        }
    }

    if (bottomMode == TAB_SETTINGS) {
        if (kDown & bottomUp) settingsSelection = (settingsSelection + 1) % 2;
        if (kDown & bottomDown) settingsSelection = (settingsSelection + 1) % 2;
        if (kDown & KEY_Y) {
            if (settingsSelection == 0) showFpsEnabled = !showFpsEnabled;
            else requestMenu = true;
        }
    } else if (bottomMode == TAB_DEBUG) {
        if (kDown & bottomUp) debugScrollPx -= 18;
        if (kDown & bottomDown) debugScrollPx += 18;

        if (kDown & KEY_TOUCH) {
            touchPosition tp;
            hidTouchRead(&tp);
            if (tp.px >= 294 && tp.py >= 44 && tp.py <= 196 && debugMaxScrollPx > 0) {
                float t = (tp.py - 44.0f) / (196.0f - 44.0f);
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                debugScrollPx = static_cast<int>(t * debugMaxScrollPx);
            }
        }

        if (debugScrollPx < 0) debugScrollPx = 0;
        if (debugScrollPx > debugMaxScrollPx) debugScrollPx = debugMaxScrollPx;
    }
}
