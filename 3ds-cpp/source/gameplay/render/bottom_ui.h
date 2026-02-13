#pragma once

#include <3ds.h>
#include <string>
#include <unordered_set>

#include "../../core/game_core.h"
#include "../../core/world_map.h"

class TextRenderer;

void renderGameplayBottomUI(
    TextRenderer& text,
    const WorldMap& world,
    const std::unordered_set<std::string>& visitedCells,
    const GameCore& core,
    int bottomMode,
    int settingsSelection,
    bool showFpsEnabled,
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
