#pragma once

#include <citro2d.h>

class TileRenderer {
public:
    bool init(const char* t3xPath, int size);
    void drawTile(float x, float y, int tileIndex) const;
    void shutdown();

private:
    void drawSlope(float x, float y, int tileIndex) const;

    C2D_SpriteSheet sheet = nullptr;
    int tileSize = 16;
};
