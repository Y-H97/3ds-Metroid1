#include "gameplay_scene.h"

void GameplayScene::setControlsSwapped(bool swapped) {
    // Tauscht Steuerungsschema zwischen Top- und Bottom-Eingaben.
    controlsSwapped = swapped;
}

void GameplayScene::setShowFpsEnabled(bool enabled) {
    // Schaltet FPS-Anzeige im Top-Screen ein/aus.
    showFpsEnabled = enabled;
}

bool GameplayScene::getShowFpsEnabled() const {
    return showFpsEnabled;
}

void GameplayScene::handleInput(u32 kDown, u32 kHeld) {
    // Übersetzt Roh-Buttons in interne Bewegungs-/Menüzustände.
    (void)kHeld;
    jumpPressed = false;

    // Je nach Option werden Steuerquellen vertauscht.
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

    // SELECT kehrt immer ins Hauptmenü zurück.
    if (kDown & KEY_SELECT) {
        requestMenu = true;
        return;
    }

    if (kDown & bottomLeft) bottomMode = (bottomMode + 3) % 4;
    if (kDown & bottomRight) bottomMode = (bottomMode + 1) % 4;

    // Touch steuert Tabs und Settings-Auswahl.
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

    // Tab-spezifische Eingabe: Inventory, Settings und Debug haben Sonderlogik.
    if (bottomMode == TAB_INVENTORY) {
        // einfache Liste basierend auf gesammelten Items
        int count = 0;
        if (collectedItems & ITEM_DOUBLE_JUMP) count++;
        if (count > 0) {
            if (kDown & bottomUp) inventorySelection = std::max(0, inventorySelection - 1);
            if (kDown & bottomDown) inventorySelection = std::min(count - 1, inventorySelection + 1);

            // toggle via button or touch tap
            bool doToggle = false;
            if (kDown & KEY_Y) doToggle = true;
            if (kDown & KEY_TOUCH) {
                touchPosition tp;
                hidTouchRead(&tp);
                // region of inventory entries: x in [16,304), y start 54 height 24 per entry
                if (tp.px >= 16 && tp.px < 304 && tp.py >= 54) {
                    int row = (tp.py - 54) / 24;
                    if (row == inventorySelection) doToggle = true;
                    else if (row >= 0 && row < count) inventorySelection = row;
                }
            }
            if (doToggle) {
                // Toggle ON/OFF
                if (inventorySelection == 0 && (collectedItems & ITEM_DOUBLE_JUMP)) {
                    if (activeItems & ITEM_DOUBLE_JUMP) {
                        activeItems &= ~ITEM_DOUBLE_JUMP;
                        setDoubleJumpEnabled(false);
                    } else {
                        activeItems |= ITEM_DOUBLE_JUMP;
                        setDoubleJumpEnabled(true);
                    }
                }
            }
        }
    } else if (bottomMode == TAB_SETTINGS) {
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
