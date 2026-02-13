#include "main_menu_controller.h"

#include "../../ui/text_renderer.h"
#include "../views/main_menu_view.h"

void MainMenuController::reset() {
    state = MENU_HOME;
    homeSelection = 0;
    playSelection = 0;
    optionsSelection = 0;
    selectedSaveSlot = 1;
    pendingAction = {};
}

void MainMenuController::setHasContinue(bool enabled) {
    hasContinue = enabled;
}

bool MainMenuController::hasContinueAvailable() const {
    return hasContinue;
}

int MainMenuController::getSelectedSaveSlot() const {
    return selectedSaveSlot;
}

void MainMenuController::setDebugEnabled(bool enabled) {
    debugEnabled = enabled;
}

bool MainMenuController::getDebugEnabled() const {
    return debugEnabled;
}

void MainMenuController::setControlsSwapped(bool enabled) {
    controlsSwapped = enabled;
}

bool MainMenuController::getControlsSwapped() const {
    return controlsSwapped;
}

void MainMenuController::handleKeys(u32 kDown) {
    if (state == MENU_PLAY) {
        if (kDown & KEY_LEFT) {
            selectedSaveSlot--;
            if (selectedSaveSlot < 1) selectedSaveSlot = SAVE_SLOT_COUNT;
        }
        if (kDown & KEY_RIGHT) {
            selectedSaveSlot++;
            if (selectedSaveSlot > SAVE_SLOT_COUNT) selectedSaveSlot = 1;
        }
    }

    if (kDown & KEY_UP) {
        if (state == MENU_HOME) homeSelection = (homeSelection + 2) % 3;
        else if (state == MENU_PLAY) playSelection = (playSelection + 2) % 3;
        else optionsSelection = (optionsSelection + 3) % 4;
    }
    if (kDown & KEY_DOWN) {
        if (state == MENU_HOME) homeSelection = (homeSelection + 1) % 3;
        else if (state == MENU_PLAY) playSelection = (playSelection + 1) % 3;
        else optionsSelection = (optionsSelection + 1) % 4;
    }

    if ((kDown & KEY_B) && state != MENU_HOME) {
        state = MENU_HOME;
    }

    if (kDown & (KEY_A | KEY_Y)) {
        if (state == MENU_HOME) {
            if (homeSelection == 0) {
                state = MENU_PLAY;
                playSelection = 0;
            } else if (homeSelection == 1) {
                state = MENU_OPTIONS;
                optionsSelection = 0;
            } else {
                pendingAction.exitGame = true;
            }
        } else if (state == MENU_PLAY) {
            if (playSelection == 0) {
                if (hasContinue) pendingAction.loadCheckpoint = true;
            } else if (playSelection == 1) {
                pendingAction.startGame = true;
            } else {
                state = MENU_HOME;
            }
        } else {
            if (optionsSelection == 0) {
                debugEnabled = !debugEnabled;
            } else if (optionsSelection == 1) {
                controlsSwapped = !controlsSwapped;
            } else if (optionsSelection == 2) {
                pendingAction.resetMapProgress = true;
            } else {
                state = MENU_HOME;
            }
        }
    }
}

void MainMenuController::handleTouch(const touchPosition& tp) {
    if (state == MENU_PLAY) {
        if (tp.py >= 10 && tp.py <= 38) {
            if (tp.px >= 44 && tp.px <= 84) {
                selectedSaveSlot--;
                if (selectedSaveSlot < 1) selectedSaveSlot = SAVE_SLOT_COUNT;
                return;
            }
            if (tp.px >= 236 && tp.px <= 276) {
                selectedSaveSlot++;
                if (selectedSaveSlot > SAVE_SLOT_COUNT) selectedSaveSlot = 1;
                return;
            }
        }
    }

    if (state == MENU_HOME) {
        if (tp.py >= 48 && tp.py <= 85) {
            homeSelection = 0;
            state = MENU_PLAY;
            playSelection = 0;
        } else if (tp.py >= 94 && tp.py <= 131) {
            homeSelection = 1;
            state = MENU_OPTIONS;
            optionsSelection = 0;
        } else if (tp.py >= 140 && tp.py <= 177) {
            homeSelection = 2;
            pendingAction.exitGame = true;
        }
    } else if (state == MENU_PLAY) {
        if (tp.py >= 48 && tp.py <= 85) {
            playSelection = 0;
            if (hasContinue) pendingAction.loadCheckpoint = true;
        } else if (tp.py >= 94 && tp.py <= 131) {
            playSelection = 1;
            pendingAction.startGame = true;
        } else if (tp.py >= 140 && tp.py <= 177) {
            playSelection = 2;
            state = MENU_HOME;
        }
    } else {
        if (tp.py >= 40 && tp.py <= 80) {
            optionsSelection = 0;
            debugEnabled = !debugEnabled;
        } else if (tp.py >= 88 && tp.py <= 128) {
            optionsSelection = 1;
            controlsSwapped = !controlsSwapped;
        } else if (tp.py >= 136 && tp.py <= 176) {
            optionsSelection = 2;
            pendingAction.resetMapProgress = true;
        } else if (tp.py >= 184 && tp.py <= 224) {
            optionsSelection = 3;
            state = MENU_HOME;
        }
    }
}

MainMenuAction MainMenuController::consumeAction() {
    MainMenuAction out = pendingAction;
    pendingAction = {};
    return out;
}

void MainMenuController::renderTop(TextRenderer& text) const {
    int activeSelection = 0;
    if (state == MENU_HOME) activeSelection = homeSelection;
    else if (state == MENU_PLAY) activeSelection = playSelection;
    else activeSelection = optionsSelection;
    drawMainMenuTopView(text, static_cast<int>(state), activeSelection, selectedSaveSlot, hasContinue, debugEnabled, controlsSwapped);
}

void MainMenuController::renderBottom(TextRenderer& text) const {
    if (state == MENU_HOME) drawMainMenuHomeView(text, homeSelection);
    else if (state == MENU_PLAY) drawMainMenuPlayView(text, playSelection, selectedSaveSlot, hasContinue);
    else drawMainMenuOptionsView(text, optionsSelection, debugEnabled, controlsSwapped);
}
