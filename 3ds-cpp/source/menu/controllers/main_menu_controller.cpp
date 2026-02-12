#include "main_menu_controller.h"

#include "../../ui/text_renderer.h"
#include "../views/main_menu_view.h"

void MainMenuController::reset() {
    state = MENU_MAIN;
    mainSelection = 0;
    optionsSelection = 0;
    pendingAction = {};
}

void MainMenuController::setDebugEnabled(bool enabled) {
    debugEnabled = enabled;
}

bool MainMenuController::getDebugEnabled() const {
    return debugEnabled;
}

void MainMenuController::handleKeys(u32 kDown) {
    if (kDown & KEY_UP) {
        if (state == MENU_MAIN) mainSelection = (mainSelection + 2) % 3;
        else optionsSelection = (optionsSelection + 1) % 2;
    }
    if (kDown & KEY_DOWN) {
        if (state == MENU_MAIN) mainSelection = (mainSelection + 1) % 3;
        else optionsSelection = (optionsSelection + 1) % 2;
    }

    if ((kDown & KEY_B) && state == MENU_OPTIONS) {
        state = MENU_MAIN;
    }

    if (kDown & KEY_A) {
        if (state == MENU_MAIN) {
            if (mainSelection == 0) {
                pendingAction.startGame = true;
            } else if (mainSelection == 1) {
                state = MENU_OPTIONS;
                optionsSelection = 0;
            } else {
                pendingAction.exitGame = true;
            }
        } else {
            if (optionsSelection == 0) {
                debugEnabled = !debugEnabled;
            } else {
                state = MENU_MAIN;
            }
        }
    }
}

void MainMenuController::handleTouch(const touchPosition& tp) {
    if (state == MENU_MAIN) {
        if (tp.py >= 48 && tp.py <= 93) {
            mainSelection = 0;
            pendingAction.startGame = true;
        } else if (tp.py >= 104 && tp.py <= 149) {
            mainSelection = 1;
            state = MENU_OPTIONS;
            optionsSelection = 0;
        } else if (tp.py >= 160 && tp.py <= 205) {
            mainSelection = 2;
            pendingAction.exitGame = true;
        }
    } else {
        if (tp.py >= 60 && tp.py <= 110) {
            optionsSelection = 0;
            debugEnabled = !debugEnabled;
        } else if (tp.py >= 124 && tp.py <= 174) {
            optionsSelection = 1;
            state = MENU_MAIN;
        }
    }
}

MainMenuAction MainMenuController::consumeAction() {
    MainMenuAction out = pendingAction;
    pendingAction = {};
    return out;
}

void MainMenuController::renderTop(TextRenderer& text) const {
    drawMainMenuTopView(text);
}

void MainMenuController::renderBottom(TextRenderer& text) const {
    if (state == MENU_MAIN) drawMainMenuMainView(text, mainSelection);
    else drawMainMenuOptionsView(text, optionsSelection, debugEnabled);
}
