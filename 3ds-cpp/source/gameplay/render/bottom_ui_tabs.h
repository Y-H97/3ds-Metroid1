#pragma once

#include <string>
#include <unordered_set>

#include "../../core/game_core.h"
#include "../../core/world_map.h"

class TextRenderer;

// Tab 0: Mini-Weltkarte mit Fog-of-War/visited-Status.
void drawBottomMapTab(
    TextRenderer& text,
    const WorldMap& world,
    const std::unordered_set<std::string>& visitedCells,
    int currentGridX,
    int currentGridY
);

// Tab 1: Inventaranzeige mit gesammelten Items und aktiven Flags.
void drawBottomInventoryTab(TextRenderer& text,
    uint32_t collectedItems,
    uint32_t activeItems,
    int selectionIndex
);

// Tab 2: Laufzeit-Einstellungen (z. B. FPS-Anzeige).
void drawBottomSettingsTab(
    TextRenderer& text,
    int settingsSelection,
    bool showFpsEnabled
);

// Tab 3: Technische Runtime-Diagnosewerte.
void drawBottomDebugTab(
    TextRenderer& text,
    const WorldMap& world,
    const GameCore& core,
    bool debugInfoEnabled,
    int currentGridX,
    int currentGridY,
    int originGridX,
    int originGridY,
    int fpsValue,
    float camX,
    float camY,
    const char* levelName,
    bool inTransition,
    bool checkpointValid,
    int checkpointGridX,
    int checkpointGridY,
    int playerTileX,
    int playerTileY,
    int playerTileId,
    int debugScrollPx,
    int& outDebugMaxScrollPx
);
