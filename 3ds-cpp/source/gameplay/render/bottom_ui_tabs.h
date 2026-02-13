#pragma once

#include <string>
#include <unordered_set>

#include "../../core/game_core.h"
#include "../../core/world_map.h"

class TextRenderer;

void drawBottomMapTab(
    TextRenderer& text,
    const WorldMap& world,
    const std::unordered_set<std::string>& visitedCells,
    int currentGridX,
    int currentGridY
);

void drawBottomInventoryTab(TextRenderer& text);

void drawBottomSettingsTab(
    TextRenderer& text,
    int settingsSelection,
    bool showFpsEnabled
);

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
