#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <string>
#include <unordered_set>

#include "../core/game_core.h"
#include "../core/world_map.h"
#include "render/tile_renderer.h"

class TextRenderer;

class GameplayScene {
public:
    static constexpr int SAVE_SLOT_COUNT = 3;

    bool init();
    void shutdown();
    void setControlsSwapped(bool swapped);
    void setShowFpsEnabled(bool enabled);
    bool getShowFpsEnabled() const;
    void setActiveSaveSlot(int slot);
    int getActiveSaveSlot() const;

    void handleInput(u32 kDown, u32 kHeld);
    void update(float dt);
    bool startNewGame(int slot);
    bool loadFromCheckpoint(int slot);
    bool hasPersistentSave(int slot);
    bool resetVisitedProgress(int slot);

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

    struct PersistentSave {
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
    bool loadPersistentSaveFromDisk(int slot, PersistentSave* outSave = nullptr);
    bool writePersistentSaveToDisk(const Checkpoint& cp, int slot);
    bool loadVisitedFromDisk(int slot);
    bool writeVisitedToDisk(int slot) const;

    GameCore core;
    WorldMap world;
    TileRenderer renderer;
    Checkpoint checkpoint;
    PersistentSave persistentSave;

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

    std::string currentLevelName;
    float lastCamX = 0.0f;
    float lastCamY = 0.0f;
    int playerTileX = 0;
    int playerTileY = 0;
    int playerTileId = 0;
    int debugScrollPx = 0;
    int debugMaxScrollPx = 0;
    bool controlsSwapped = false;
    bool moveLeftHeld = false;
    bool moveRightHeld = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    int activeSaveSlot = 1;
    std::unordered_set<std::string> visitedCells;
};
