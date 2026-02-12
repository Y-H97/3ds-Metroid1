#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <string>

#include "../core/game_core.h"
#include "../core/world_map.h"
#include "render/tile_renderer.h"

class TextRenderer;

class GameplayScene {
public:
    bool init();
    void shutdown();

    void handleInput(u32 kDown, u32 kHeld);
    void update(float dt);

    void renderTop(C3D_RenderTarget* top, TextRenderer& text, bool debugInfoEnabled);
    void renderBottom(C3D_RenderTarget* bottom, TextRenderer& text, bool debugInfoEnabled);

    bool shouldExitGame() const;
    bool shouldReturnToMenu() const;
    void clearReturnToMenu();

private:
    struct Checkpoint {
        bool valid = false;
        std::string level;
        int gridX = 0;
        int gridY = 0;
        float x = 0.0f;
        float y = 0.0f;
    };

    enum BottomTab {
        TAB_MAP = 0,
        TAB_INVENTORY = 1,
        TAB_SETTINGS = 2,
        TAB_DEBUG = 3,
    };

    bool loadInitialMap();

    GameCore core;
    WorldMap world;
    TileRenderer renderer;
    Checkpoint checkpoint;

    int gridX = 0;
    int gridY = 0;
    int currentGridX = 0;
    int currentGridY = 0;

    int bottomMode = TAB_MAP;
    int settingsSelection = 0;
    bool showFpsEnabled = false;
    bool inTransition = false;
    bool requestExit = false;
    bool requestMenu = false;

    u64 fpsTimerMs = 0;
    int fpsFrameCounter = 0;
    int fpsValue = 0;
};
