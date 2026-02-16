#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

namespace {

std::string makeVisitedKey(int x, int y) {
    // Einheitlicher Schlüssel für besuchte Kartenzellen.
    return std::to_string(x) + "," + std::to_string(y);
}

} // namespace

void drawBottomMapTab(
    TextRenderer& text,
    const WorldMap& world,
    const std::unordered_set<std::string>& visitedCells,
    int currentGridX,
    int currentGridY
) {
    // Zeichnet eine abstrahierte Weltkarte: besucht, aktuell, Ursprung je Segment.
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
}
