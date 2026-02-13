#include "bottom_ui.h"

#include <citro2d.h>

static std::string makeVisitedKey(int x, int y) {
    return std::to_string(x) + "," + std::to_string(y);
}

#include "../../ui/text_renderer.h"

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
) {
    outDebugMaxScrollPx = 0;
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(20, 20, 30, 255));
    C2D_DrawRectSolid(6, 8, 0.0f, 308, 208, C2D_Color32(18, 24, 34, 255));

    if (bottomMode == 0) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Karte");
        text.draw(214.0f, 12.0f, 0.36f, C2D_Color32(170, 180, 205, 255), "GX:%d GY:%d", currentGridX, currentGridY);

        int minX = 0;
        int minY = 0;
        int maxX = 0;
        int maxY = 0;
        if (world.getSpatialBounds(minX, minY, maxX, maxY)) {
            int cols = (maxX - minX) + 1;
            int rows = (maxY - minY) + 1;
            int panelX = 12;
            int panelY = 28;
            int panelW = 296;
            int panelH = 180;
            int cellW = panelW / (cols > 0 ? cols : 1);
            int cellH = panelH / (rows > 0 ? rows : 1);
            int cellSize = cellW < cellH ? cellW : cellH;
            if (cellSize < 6) cellSize = 6;
            if (cellSize > 20) cellSize = 20;

            int drawW = cols * cellSize;
            int drawH = rows * cellSize;
            int originX = panelX + (panelW - drawW) / 2;
            int originY = panelY + (panelH - drawH) / 2;
            C2D_DrawRectSolid(originX - 2, originY - 2, 0.0f, drawW + 4, drawH + 4, C2D_Color32(36, 46, 62, 255));

            for (int gy = minY; gy <= maxY; ++gy) {
                for (int gx = minX; gx <= maxX; ++gx) {
                    const SpatialCell* sc = world.getCell(gx, gy);
                    if (!sc) continue;

                    int cx = originX + (gx - minX) * cellSize;
                    int cy = originY + (gy - minY) * cellSize;
                    bool isCurrent = (gx == currentGridX && gy == currentGridY);
                    bool isVisited = visitedCells.find(makeVisitedKey(gx, gy)) != visitedCells.end();
                    if (!isVisited && !isCurrent) continue;
                    bool isOrigin = (gx == sc->originX && gy == sc->originY);
                    u32 color = isCurrent ? C2D_Color32(100, 220, 130, 255) : (isOrigin ? C2D_Color32(102, 150, 210, 255) : C2D_Color32(76, 112, 164, 255));
                    C2D_DrawRectSolid(cx, cy, 0.0f, cellSize - 1, cellSize - 1, color);
                }
            }
        }
    } else if (bottomMode == 1) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Inventar");
        C2D_DrawRectSolid(16, 44, 0.0f, 288, 144, C2D_Color32(28, 34, 46, 255));
        text.draw(44.0f, 108.0f, 0.46f, C2D_Color32(170, 180, 205, 255), "(Aktuell leer)");
    } else if (bottomMode == 2) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Einstellungen");
        u32 s0 = settingsSelection == 0 ? C2D_Color32(76, 112, 182, 255) : C2D_Color32(46, 62, 92, 255);
        u32 s1 = settingsSelection == 1 ? C2D_Color32(182, 84, 84, 255) : C2D_Color32(92, 46, 46, 255);
        C2D_DrawRectSolid(20, 54, 0.0f, 280, 42, s0);
        C2D_DrawRectSolid(20, 104, 0.0f, 280, 42, s1);
        text.draw(28.0f, 67.0f, 0.40f, C2D_Color32(236, 236, 245, 255), "FPS Anzeige");
        text.draw(248.0f, 67.0f, 0.44f, showFpsEnabled ? C2D_Color32(130, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), showFpsEnabled ? "AN" : "AUS");
        text.draw(28.0f, 117.0f, 0.39f, C2D_Color32(245, 236, 236, 255), "Zum Hauptmenue");
    } else {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Debug");
        C2D_DrawRectSolid(16, 38, 0.0f, 288, 166, C2D_Color32(24, 30, 42, 255));
        if (!debugInfoEnabled) {
            text.draw(26.0f, 110.0f, 0.42f, C2D_Color32(180, 190, 210, 255), "Debug ist im Hauptmenue AUS");
        } else {
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
    }

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
