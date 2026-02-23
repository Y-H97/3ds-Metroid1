#include "bottom_ui.h"
#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

void renderGameplayBottomUI(
    TextRenderer& text,
    const WorldMap& world,
    const std::unordered_set<std::string>& visitedCells,
    const GameCore& core,
    uint32_t collectedItems,
    uint32_t activeItems,
    int inventorySelection,
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
) {
    // Wird vom aktiven Tab gesetzt, falls Scrollbereich existiert (Debug-Tab).
    outDebugMaxScrollPx = 0;
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(20, 20, 30, 255));
    C2D_DrawRectSolid(6, 8, 0.0f, 308, 208, C2D_Color32(18, 24, 34, 255));

    // Inhalt je nach aktivem Tab zeichnen.
    if (bottomMode == 0) {
        drawBottomMapTab(text, world, visitedCells, currentGridX, currentGridY);
    } else if (bottomMode == 1) {
        drawBottomInventoryTab(text, collectedItems, activeItems, inventorySelection);
    } else if (bottomMode == 2) {
        drawBottomSettingsTab(text, settingsSelection, showFpsEnabled);
    } else {
        drawBottomDebugTab(
            text,
            world,
            core,
            debugInfoEnabled,
            currentGridX,
            currentGridY,
            originGridX,
            originGridY,
            fpsValue,
            camX,
            camY,
            levelName,
            inTransition,
            checkpointValid,
            checkpointGridX,
            checkpointGridY,
            playerTileX,
            playerTileY,
            playerTileId,
            debugScrollPx,
            outDebugMaxScrollPx
        );
    }

    // Untere Tab-Leiste (Map/Inv/Set/Dbg).
    u32 tabBg = C2D_Color32(30, 30, 45, 255);
    C2D_DrawRectSolid(0, 220, 0.0f, 320, 20, tabBg);
    const char* tabNames[4] = {"Map", "Inv", "Set", "Dbg"};
    for (int i = 0; i < 4; ++i) {
        bool selected = (i == bottomMode);
        u32 col = selected ? C2D_Color32(80, 140, 220, 255) : C2D_Color32(50, 50, 70, 255);
        C2D_DrawRectSolid(i * 80, 220, 0.0f, 80, 20, col);
        text.draw(i * 80.0f + 28.0f, 224.0f, 0.34f, C2D_Color32(235, 235, 245, 255), "%s", tabNames[i]);
    }
}
