#include "main_menu_controller.h"

#ifndef DESKTOP_SIMULATOR
#include "../../ui/text_renderer.h"
#include "../views/main_menu_view.h"
#endif
#include "../main_menu_layout.h"
#include "../views/manual_content.h"

int MainMenuController::getManualMaxScroll() const {
    // Maximale Scrollposition der aktuellen Handbuchseite berechnen.
    return manualMaxScrollForViewport(manualSelection, 12);
}

void MainMenuController::reset() {
    // Alle Menü- und Auswahlzustände auf Standard zurücksetzen.
    state = MENU_HOME;
    homeSelection = 0;
    playSelection = 0;
    optionsSelection = 0;
    manualSelection = 0;
    manualScroll = 0;
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

void MainMenuController::handleHomeKeys(u32 kDown) {
    // Navigation im Hauptmenü (Kategorien).
    if (kDown & KEY_UP) homeSelection = (homeSelection + 3) % 4;
    if (kDown & KEY_DOWN) homeSelection = (homeSelection + 1) % 4;

    if (kDown & (KEY_A | KEY_Y)) {
        if (homeSelection == 0) {
            state = MENU_PLAY;
            playSelection = 0;
        } else if (homeSelection == 1) {
            state = MENU_MANUAL_LIST;
            manualSelection = 0;
            manualScroll = 0;
        } else if (homeSelection == 2) {
            state = MENU_OPTIONS;
            optionsSelection = 0;
        } else {
            pendingAction.exitGame = true;
        }
    }
}

void MainMenuController::handlePlayKeys(u32 kDown) {
    // Spiel-Untermenü: Slot wählen, Fortsetzen/Neu/Zurück.
    if (kDown & KEY_LEFT) {
        selectedSaveSlot--;
        if (selectedSaveSlot < 1) selectedSaveSlot = SAVE_SLOT_COUNT;
    }
    if (kDown & KEY_RIGHT) {
        selectedSaveSlot++;
        if (selectedSaveSlot > SAVE_SLOT_COUNT) selectedSaveSlot = 1;
    }

    if (kDown & KEY_UP) playSelection = (playSelection + 2) % 3;
    if (kDown & KEY_DOWN) playSelection = (playSelection + 1) % 3;

    if (kDown & (KEY_A | KEY_Y)) {
        if (playSelection == 0) {
            if (hasContinue) pendingAction.loadCheckpoint = true;
        } else if (playSelection == 1) {
            pendingAction.startGame = true;
        } else {
            state = MENU_HOME;
        }
    }
}

void MainMenuController::handleOptionsKeys(u32 kDown) {
    // Optionen-Untermenü: Flags toggeln und Aktionen auslösen.
    if (kDown & KEY_UP) optionsSelection = (optionsSelection + 3) % 4;
    if (kDown & KEY_DOWN) optionsSelection = (optionsSelection + 1) % 4;

    if (kDown & (KEY_A | KEY_Y)) {
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

void MainMenuController::handleManualListKeys(u32 kDown) {
    // Handbuch-Themenliste bedienen.
    int topicCount = manualTopicCount();
    if (kDown & KEY_UP) manualSelection = (manualSelection + topicCount) % (topicCount + 1);
    if (kDown & KEY_DOWN) manualSelection = (manualSelection + 1) % (topicCount + 1);

    if (kDown & (KEY_A | KEY_Y)) {
        if (manualSelection < topicCount) {
            state = MENU_MANUAL_PAGE;
            manualScroll = 0;
        } else {
            state = MENU_HOME;
        }
    }
}

void MainMenuController::handleManualPageKeys(u32 kDown) {
    // Scrollen innerhalb einer Handbuchseite.
    if (kDown & KEY_UP) {
        manualScroll -= 1;
        if (manualScroll < 0) manualScroll = 0;
    }
    if (kDown & KEY_DOWN) {
        manualScroll += 1;
        int maxScroll = getManualMaxScroll();
        if (manualScroll > maxScroll) manualScroll = maxScroll;
    }
    if (kDown & (KEY_A | KEY_Y)) {
        state = MENU_MANUAL_LIST;
    }
}

void MainMenuController::handleHomeTouch(const touchPosition& tp) {
    // Touch-Hotspots im Hauptmenü.
    if (mainmenu::layout::contains(mainmenu::layout::homeButtonRect(0), tp.px, tp.py)) {
        homeSelection = 0;
        state = MENU_PLAY;
        playSelection = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::homeButtonRect(1), tp.px, tp.py)) {
        homeSelection = 1;
        state = MENU_MANUAL_LIST;
        manualSelection = 0;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::homeButtonRect(2), tp.px, tp.py)) {
        homeSelection = 2;
        state = MENU_OPTIONS;
        optionsSelection = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::homeButtonRect(3), tp.px, tp.py)) {
        homeSelection = 3;
        pendingAction.exitGame = true;
    }
}

void MainMenuController::handlePlayTouch(const touchPosition& tp) {
    // Touch-Hotspots im Spiel-Untermenü inkl. Slot-Pfeile.
    if (mainmenu::layout::contains(mainmenu::layout::playSlotLeftRect(), tp.px, tp.py)) {
            selectedSaveSlot--;
            if (selectedSaveSlot < 1) selectedSaveSlot = SAVE_SLOT_COUNT;
            return;
    }
    if (mainmenu::layout::contains(mainmenu::layout::playSlotRightRect(), tp.px, tp.py)) {
            selectedSaveSlot++;
            if (selectedSaveSlot > SAVE_SLOT_COUNT) selectedSaveSlot = 1;
            return;
    }

    if (mainmenu::layout::contains(mainmenu::layout::playButtonRect(0), tp.px, tp.py)) {
        playSelection = 0;
        if (hasContinue) pendingAction.loadCheckpoint = true;
    } else if (mainmenu::layout::contains(mainmenu::layout::playButtonRect(1), tp.px, tp.py)) {
        playSelection = 1;
        pendingAction.startGame = true;
    } else if (mainmenu::layout::contains(mainmenu::layout::playButtonRect(2), tp.px, tp.py)) {
        playSelection = 2;
        state = MENU_HOME;
    }
}

void MainMenuController::handleOptionsTouch(const touchPosition& tp) {
    // Touch-Hotspots in den Optionen.
    if (mainmenu::layout::contains(mainmenu::layout::optionsButtonRect(0), tp.px, tp.py)) {
        optionsSelection = 0;
        debugEnabled = !debugEnabled;
    } else if (mainmenu::layout::contains(mainmenu::layout::optionsButtonRect(1), tp.px, tp.py)) {
        optionsSelection = 1;
        controlsSwapped = !controlsSwapped;
    } else if (mainmenu::layout::contains(mainmenu::layout::optionsButtonRect(2), tp.px, tp.py)) {
        optionsSelection = 2;
        pendingAction.resetMapProgress = true;
    } else if (mainmenu::layout::contains(mainmenu::layout::optionsButtonRect(3), tp.px, tp.py)) {
        optionsSelection = 3;
        state = MENU_HOME;
    }
}

void MainMenuController::handleManualListTouch(const touchPosition& tp) {
    // Touch-Hotspots in der Themenliste.
    if (mainmenu::layout::contains(mainmenu::layout::manualTopicRect(0), tp.px, tp.py)) {
        manualSelection = 0;
        state = MENU_MANUAL_PAGE;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualTopicRect(1), tp.px, tp.py)) {
        manualSelection = 1;
        state = MENU_MANUAL_PAGE;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualTopicRect(2), tp.px, tp.py)) {
        manualSelection = 2;
        state = MENU_MANUAL_PAGE;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualTopicRect(3), tp.px, tp.py)) {
        manualSelection = 3;
        state = MENU_MANUAL_PAGE;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualTopicRect(4), tp.px, tp.py)) {
        manualSelection = 4;
        state = MENU_MANUAL_PAGE;
        manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualBackRect(), tp.px, tp.py)) {
        manualSelection = manualTopicCount();
        state = MENU_HOME;
    }
}

void MainMenuController::handleManualPageTouch(const touchPosition& tp) {
    // Touch-Hotspots für Scroll-Zonen und Zurück.
    if (mainmenu::layout::contains(mainmenu::layout::manualPageScrollUpRect(), tp.px, tp.py)) {
        manualScroll -= 1;
        if (manualScroll < 0) manualScroll = 0;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualBackRect(), tp.px, tp.py)) {
        state = MENU_MANUAL_LIST;
    } else if (mainmenu::layout::contains(mainmenu::layout::manualPageScrollDownRect(), tp.px, tp.py)) {
        manualScroll += 1;
        int maxScroll = getManualMaxScroll();
        if (manualScroll > maxScroll) manualScroll = maxScroll;
    }
}

void MainMenuController::handleKeys(u32 kDown) {
    // Zentrale Tastenverteilung abhängig vom aktuellen Menüzustand.
    if ((kDown & KEY_B) && state != MENU_HOME) {
        if (state == MENU_MANUAL_PAGE) {
            state = MENU_MANUAL_LIST;
        } else {
            state = MENU_HOME;
        }
        return;
    }

    if (state == MENU_HOME) handleHomeKeys(kDown);
    else if (state == MENU_PLAY) handlePlayKeys(kDown);
    else if (state == MENU_OPTIONS) handleOptionsKeys(kDown);
    else if (state == MENU_MANUAL_LIST) handleManualListKeys(kDown);
    else handleManualPageKeys(kDown);
}

void MainMenuController::handleTouch(const touchPosition& tp) {
    // Zentrale Touchverteilung abhängig vom aktuellen Menüzustand.
    if (state == MENU_HOME) handleHomeTouch(tp);
    else if (state == MENU_PLAY) handlePlayTouch(tp);
    else if (state == MENU_OPTIONS) handleOptionsTouch(tp);
    else if (state == MENU_MANUAL_LIST) handleManualListTouch(tp);
    else handleManualPageTouch(tp);
}

MainMenuAction MainMenuController::consumeAction() {
    // Aktionen als einmaliges Event abholen.
    MainMenuAction out = pendingAction;
    pendingAction = {};
    return out;
}

void MainMenuController::renderTop(TextRenderer& text) const {
    // Top-Screen-Rendering an View-Modul delegieren.
#ifndef DESKTOP_SIMULATOR
    int activeSelection = 0;
    if (state == MENU_HOME) activeSelection = homeSelection;
    else if (state == MENU_PLAY) activeSelection = playSelection;
    else if (state == MENU_MANUAL_LIST || state == MENU_MANUAL_PAGE) activeSelection = manualSelection;
    else activeSelection = optionsSelection;
    drawMainMenuTopView(text, static_cast<int>(state), activeSelection, selectedSaveSlot, hasContinue, debugEnabled, controlsSwapped, manualScroll);
#else
    (void)text;
#endif
}

void MainMenuController::renderBottom(TextRenderer& text) const {
    // Bottom-Screen-Rendering passend zum Menüzustand delegieren.
#ifndef DESKTOP_SIMULATOR
    if (state == MENU_HOME) drawMainMenuHomeView(text, homeSelection);
    else if (state == MENU_PLAY) drawMainMenuPlayView(text, playSelection, selectedSaveSlot, hasContinue);
    else if (state == MENU_MANUAL_LIST) drawMainMenuManualListView(text, manualSelection);
    else if (state == MENU_MANUAL_PAGE) drawMainMenuManualPageControlsView(text, manualSelection, manualScroll, getManualMaxScroll());
    else drawMainMenuOptionsView(text, optionsSelection, debugEnabled, controlsSwapped);
#else
    (void)text;
#endif
}

MainMenuState MainMenuController::getState() const {
    return state;
}

int MainMenuController::getHomeSelection() const {
    return homeSelection;
}

int MainMenuController::getPlaySelection() const {
    return playSelection;
}

int MainMenuController::getOptionsSelection() const {
    return optionsSelection;
}

int MainMenuController::getManualSelection() const {
    return manualSelection;
}

int MainMenuController::getManualScroll() const {
    return manualScroll;
}
