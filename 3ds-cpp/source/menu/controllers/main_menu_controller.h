#pragma once

#ifdef DESKTOP_SIMULATOR
#include "../../../simulator/compat/desktop_3ds_compat.h"
#else
#include <3ds.h>
#endif

class TextRenderer;

// Zustände des Hauptmenüs.
// HOME: Kategorien, PLAY: Spielaktionen, OPTIONS: Einstellungen,
// MANUAL_LIST: Themenübersicht, MANUAL_PAGE: scrollbare Detailseite.
enum MainMenuState {
    MENU_HOME = 0,
    MENU_PLAY = 1,
    MENU_OPTIONS = 2,
    MENU_MANUAL_LIST = 3,
    MENU_MANUAL_PAGE = 4,
};

struct MainMenuAction {
    bool startGame = false;
    bool loadCheckpoint = false;
    bool resetMapProgress = false;
    bool exitGame = false;
};

class MainMenuController {
public:
    static constexpr int SAVE_SLOT_COUNT = 3;

    void reset();
    void setHasContinue(bool enabled);
    bool hasContinueAvailable() const;
    int getSelectedSaveSlot() const;
    void setDebugEnabled(bool enabled);
    bool getDebugEnabled() const;
    void setControlsSwapped(bool enabled);
    bool getControlsSwapped() const;

    void handleKeys(u32 kDown);
    void handleTouch(const touchPosition& tp);
    MainMenuAction consumeAction();

    void renderTop(TextRenderer& text) const;
    void renderBottom(TextRenderer& text) const;

    MainMenuState getState() const;
    int getHomeSelection() const;
    int getPlaySelection() const;
    int getOptionsSelection() const;
    int getManualSelection() const;
    int getManualScroll() const;

private:
    // Bestimmt die Scrollgrenze für die aktuelle Handbuchseite
    // bei der aktuellen Textfensterhöhe auf dem Top-Screen.
    int getManualMaxScroll() const;

    void handleHomeKeys(u32 kDown);
    void handlePlayKeys(u32 kDown);
    void handleOptionsKeys(u32 kDown);
    void handleManualListKeys(u32 kDown);
    void handleManualPageKeys(u32 kDown);

    void handleHomeTouch(const touchPosition& tp);
    void handlePlayTouch(const touchPosition& tp);
    void handleOptionsTouch(const touchPosition& tp);
    void handleManualListTouch(const touchPosition& tp);
    void handleManualPageTouch(const touchPosition& tp);

    MainMenuState state = MENU_HOME;
    int homeSelection = 0;
    int playSelection = 0;
    int optionsSelection = 0;
    int manualSelection = 0;
    int manualScroll = 0;
    int selectedSaveSlot = 1;
    bool hasContinue = false;
    bool debugEnabled = true;
    bool controlsSwapped = false;
    MainMenuAction pendingAction;
};
