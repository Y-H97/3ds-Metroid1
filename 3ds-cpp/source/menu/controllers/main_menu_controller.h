#pragma once

#include <3ds.h>

class TextRenderer;

enum MainMenuState {
    MENU_HOME = 0,
    MENU_PLAY = 1,
    MENU_OPTIONS = 2,
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

private:
    MainMenuState state = MENU_HOME;
    int homeSelection = 0;
    int playSelection = 0;
    int optionsSelection = 0;
    int selectedSaveSlot = 1;
    bool hasContinue = false;
    bool debugEnabled = true;
    bool controlsSwapped = false;
    MainMenuAction pendingAction;
};
