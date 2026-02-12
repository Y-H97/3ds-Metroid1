#include "bottom_ui.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

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
) {
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
        text.draw(28.0f, 117.0f, 0.44f, C2D_Color32(245, 236, 236, 255), "Spiel verlassen");
    } else {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Debug");
        C2D_DrawRectSolid(16, 38, 0.0f, 288, 166, C2D_Color32(24, 30, 42, 255));
        if (!debugInfoEnabled) {
            text.draw(26.0f, 110.0f, 0.42f, C2D_Color32(180, 190, 210, 255), "Debug ist im Hauptmenue AUS");
        } else {
            const Player& p = core.getPlayer();
            text.draw(24.0f, 48.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Player X/Y: %.1f / %.1f", p.x, p.y);
            text.draw(24.0f, 68.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Vx/Vy: %.1f / %.1f", p.vx, p.vy);
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
