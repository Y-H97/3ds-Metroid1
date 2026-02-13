#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

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
) {
    text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Debug");
    C2D_DrawRectSolid(16, 38, 0.0f, 288, 166, C2D_Color32(24, 30, 42, 255));
    if (!debugInfoEnabled) {
        text.draw(26.0f, 110.0f, 0.42f, C2D_Color32(180, 190, 210, 255), "Debug ist im Hauptmenue AUS");
        return;
    }

    const Player& p = core.getPlayer();
    const TileMap& map = core.getMap();
    const float viewportTop = 44.0f;
    const float viewportBottom = 196.0f;
    const float lineHeight = 18.0f;
    const int lineCount = 13;
    const int contentHeight = static_cast<int>(lineCount * lineHeight);
    const int viewportHeight = static_cast<int>(viewportBottom - viewportTop);
    int maxScroll = contentHeight - viewportHeight;
    if (maxScroll < 0) maxScroll = 0;
    outDebugMaxScrollPx = maxScroll;

    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    bool hasBounds = world.getSpatialBounds(minX, minY, maxX, maxY);
    int transitions = static_cast<int>(map.getTransitions().size());
    int worldCells = static_cast<int>(world.getCells().size());
    int mapPixelsW = map.width * 16;
    int mapPixelsH = map.height * 16;

    auto drawIfVisible = [&](float y, const char* fmt, auto... vals) {
        if (y < viewportTop || y > viewportBottom) return;
        text.draw(24.0f, y, 0.34f, C2D_Color32(190, 210, 235, 255), fmt, vals...);
    };

    float y = 44.0f - static_cast<float>(debugScrollPx);
    drawIfVisible(y, "Lvl:%s  FPS:%d", levelName ? levelName : "?", fpsValue); y += lineHeight;
    drawIfVisible(y, "Map Tiles:%dx%d  Px:%dx%d", map.width, map.height, mapPixelsW, mapPixelsH); y += lineHeight;
    drawIfVisible(y, "Grid Cur:%d,%d  Origin:%d,%d", currentGridX, currentGridY, originGridX, originGridY); y += lineHeight;
    drawIfVisible(y, "World Cells:%d  Bounds:%s", worldCells, hasBounds ? "OK" : "NONE"); y += lineHeight;
    if (hasBounds) {
        drawIfVisible(y, "Bound Min:%d,%d  Max:%d,%d", minX, minY, maxX, maxY);
    } else {
        drawIfVisible(y, "Bound Min/Max: -");
    }
    y += lineHeight;
    drawIfVisible(y, "Player Pos:%.1f / %.1f", p.x, p.y); y += lineHeight;
    drawIfVisible(y, "Player Vel:%.1f / %.1f", p.vx, p.vy); y += lineHeight;
    drawIfVisible(y, "Player Size:%.1f x %.1f", p.w, p.h); y += lineHeight;
    drawIfVisible(y, "Tile X/Y:%d/%d  ID:%d", playerTileX, playerTileY, playerTileId); y += lineHeight;
    drawIfVisible(y, "Cam X/Y:%.1f / %.1f", camX, camY); y += lineHeight;
    drawIfVisible(y, "Ground:%s  Coyote:%.2f", p.grounded ? "Y" : "N", p.coyoteTimer); y += lineHeight;
    drawIfVisible(y, "Transition:%s  Count:%d", inTransition ? "Y" : "N", transitions); y += lineHeight;
    drawIfVisible(y, "Checkpoint:%s (%d,%d)", checkpointValid ? "Y" : "N", checkpointGridX, checkpointGridY);

    if (maxScroll > 0) {
        const float trackX = 296.0f;
        const float trackY = 44.0f;
        const float trackW = 4.0f;
        const float trackH = viewportBottom - viewportTop;
        C2D_DrawRectSolid(trackX, trackY, 0.0f, trackW, trackH, C2D_Color32(58, 72, 96, 255));

        float thumbH = (trackH * trackH) / static_cast<float>(contentHeight);
        if (thumbH < 18.0f) thumbH = 18.0f;
        float t = static_cast<float>(debugScrollPx) / static_cast<float>(maxScroll);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float thumbY = trackY + t * (trackH - thumbH);
        C2D_DrawRectSolid(trackX, thumbY, 0.0f, trackW, thumbH, C2D_Color32(132, 176, 236, 255));
    }
}
