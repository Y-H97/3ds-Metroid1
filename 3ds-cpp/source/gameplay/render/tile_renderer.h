#pragma once

#ifndef DESKTOP_SIMULATOR
#include <citro2d.h>
#else
// stub definitions for desktop build
using C2D_SpriteSheet = void*;
#endif

class TileRenderer {
public:
    // Lädt Tileset (T3X) und setzt Kachelgröße.
    bool init(const char* t3xPath, int size);
    // Zeichnet eine einzelne Tile-ID an Pixelposition x/y.
    void drawTile(float x, float y, int tileIndex) const;
    // Gibt SpriteSheet-Ressourcen frei.
    void shutdown();

private:
    // Spezialrenderer für Schräge-Tiles (30..33).
    void drawSlope(float x, float y, int tileIndex) const;

    C2D_SpriteSheet sheet = nullptr;
    int tileSize = 16;
};
