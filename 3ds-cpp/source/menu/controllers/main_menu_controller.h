#pragma once

#include <3ds.h>

class TextRenderer;

enum MainMenuState {
    MENU_MAIN = 0,
    MENU_OPTIONS = 1,
};

struct MainMenuAction {
    bool startGame = false;
    bool exitGame = false;
};

class MainMenuController {
public:
    void reset();
    void setDebugEnabled(bool enabled);
    bool getDebugEnabled() const;

    void handleKeys(u32 kDown);
    void handleTouch(const touchPosition& tp);
    MainMenuAction consumeAction();

    void renderTop(TextRenderer& text) const;
    void renderBottom(TextRenderer& text) const;

private:
    MainMenuState state = MENU_MAIN;
    int mainSelection = 0;
    int optionsSelection = 0;
    bool debugEnabled = true;
    MainMenuAction pendingAction;
};
