#pragma once

#include <3ds.h>

#include "../../core/game_core.h"
#include "../../core/world_map.h"

class TextRenderer;

void renderGameplayBottomUI(
    TextRenderer& text,
    const WorldMap& world,
    const GameCore& core,
    int bottomMode,
    int settingsSelection,
    bool showFpsEnabled,
    bool debugInfoEnabled,
    int currentGridX,
    int currentGridY
);
