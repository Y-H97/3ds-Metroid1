#include "tile_renderer.h"

constexpr u32 TILE_COLOR  = C2D_Color32(90, 180, 220, 255);

bool TileRenderer::init(const char* t3xPath, int size) {
    // SpriteSheet laden; bei Fehlschlag wird später auf Farbflächen zurückgefallen.
    tileSize = size;
    sheet = C2D_SpriteSheetLoad(t3xPath);
    return sheet != nullptr;
}

void TileRenderer::drawSlope(float x, float y, int tileIndex) const {
    // Schräge Kacheln werden geometrisch per Dreiecken gezeichnet.
    u32 colMain = C2D_Color32(153, 153, 230, 255);
    u32 colShade = C2D_Color32(118, 118, 188, 255);
    u32 lineCol = C2D_Color32(198, 198, 250, 255);
    if (tileIndex == 32 || tileIndex == 33) {
        colMain = C2D_Color32(122, 122, 194, 255);
        colShade = C2D_Color32(94, 94, 156, 255);
        lineCol = C2D_Color32(170, 170, 228, 255);
    }
    const float s = static_cast<float>(tileSize);
    C2D_DrawRectSolid(x, y, 0.0f, s, s, C2D_Color32(28, 34, 50, 255));
    if (tileIndex == 30) {
        C2D_DrawTriangle(x, y + s, colMain, x + s, y + s, colMain, x + s, y, colMain, 0.05f);
        C2D_DrawTriangle(x + s * 0.5f, y + s, colShade, x + s, y + s, colShade, x + s, y + s * 0.5f, colShade, 0.06f);
        C2D_DrawLine(x + s, y, lineCol, x, y + s, lineCol, 1.4f, 0.08f);
    } else if (tileIndex == 31) {
        C2D_DrawTriangle(x, y, colMain, x, y + s, colMain, x + s, y + s, colMain, 0.05f);
        C2D_DrawTriangle(x, y + s * 0.5f, colShade, x, y + s, colShade, x + s * 0.5f, y + s, colShade, 0.06f);
        C2D_DrawLine(x, y, lineCol, x + s, y + s, lineCol, 1.4f, 0.08f);
    } else if (tileIndex == 32) {
        C2D_DrawTriangle(x, y, colMain, x + s, y, colMain, x + s, y + s, colMain, 0.05f);
        C2D_DrawTriangle(x + s * 0.5f, y, colShade, x + s, y, colShade, x + s, y + s * 0.5f, colShade, 0.06f);
        C2D_DrawLine(x, y, lineCol, x + s, y + s, lineCol, 1.4f, 0.08f);
    } else if (tileIndex == 33) {
        C2D_DrawTriangle(x, y, colMain, x + s, y, colMain, x, y + s, colMain, 0.05f);
        C2D_DrawTriangle(x, y, colShade, x + s * 0.5f, y, colShade, x, y + s * 0.5f, colShade, 0.06f);
        C2D_DrawLine(x + s, y, lineCol, x, y + s, lineCol, 1.4f, 0.08f);
    }
}

void TileRenderer::drawTile(float x, float y, int tileIndex) const {
    // Schräge haben ein eigenes Zeichnungsverhalten.
    if (tileIndex == 30 || tileIndex == 31 || tileIndex == 32 || tileIndex == 33) {
        drawSlope(x, y, tileIndex);
        return;
    }

    // Primär: aus SpriteSheet zeichnen.
    if (sheet) {
        C2D_Image img = C2D_SpriteSheetGetImage(sheet, tileIndex % C2D_SpriteSheetCount(sheet));
        C2D_DrawImageAt(img, x, y, 0.0f, nullptr, 1.0f, 1.0f);
    } else {
        // Fallback ohne SpriteSheet: farbige Rechtecke nach Tile-Typ.
        u32 col = TILE_COLOR;
        if (tileIndex == 1) col = C2D_Color32(128, 128, 204, 255);
        else if (tileIndex == 2) col = C2D_Color32(230, 51, 51, 255);
        else if (tileIndex >= 30) {
            if (tileIndex == 30 || tileIndex == 31) col = C2D_Color32(153, 153, 230, 255);
            else col = C2D_Color32(102, 102, 179, 255);
        } else if (tileIndex >= 4) {
            switch (tileIndex) {
                case 4: col = C2D_Color32(51, 51, 51, 255); break;
                case 5: col = C2D_Color32(26, 26, 77, 255); break;
                case 6: col = C2D_Color32(26, 51, 26, 255); break;
                case 7: col = C2D_Color32(51, 26, 51, 255); break;
                case 8: col = C2D_Color32(38, 38, 38, 255); break;
                case 9: col = C2D_Color32(20, 20, 20, 255); break;
                case 10: col = C2D_Color32(46, 56, 64, 255); break;
                case 11: col = C2D_Color32(13, 13, 51, 255); break;
                case 12: col = C2D_Color32(0, 51, 64, 255); break;
                case 13: col = C2D_Color32(13, 38, 13, 255); break;
                case 14: col = C2D_Color32(51, 51, 26, 255); break;
                case 15: col = C2D_Color32(77, 26, 26, 255); break;
                case 16: col = C2D_Color32(77, 51, 26, 255); break;
                case 17: col = C2D_Color32(51, 38, 20, 255); break;
                case 18: col = C2D_Color32(64, 26, 13, 255); break;
                case 19: col = C2D_Color32(0, 64, 64, 255); break;
                case 20: col = C2D_Color32(38, 46, 31, 255); break;
                case 21: col = C2D_Color32(89, 38, 26, 255); break;
                default: col = TILE_COLOR; break;
            }
        }
        C2D_DrawRectSolid(x, y, 0.0f, static_cast<float>(tileSize), static_cast<float>(tileSize), col);
    }
}

void TileRenderer::shutdown() {
    // Grafikressourcen freigeben.
    if (sheet) {
        C2D_SpriteSheetFree(sheet);
        sheet = nullptr;
    }
}
