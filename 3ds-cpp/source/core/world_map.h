#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct WorldCell {
    int x = 0;
    int y = 0;
    std::string level;
};

struct SpatialCell {
    std::string level;
    int originX = 0;
    int originY = 0;
};

class WorldMap {
public:
    bool loadWorldJson(const char* path);
    bool buildSpatialMap(const char* mapsRoot, int tileSize, int screenW, int screenH);

    const std::vector<WorldCell>& getCells() const { return cells; }
    const SpatialCell* getCell(int gridX, int gridY) const;
    bool getSpatialBounds(int& minX, int& minY, int& maxX, int& maxY) const;

    bool isCheckpoint(int gridX, int gridY) const;

private:
    std::vector<WorldCell> cells;
    std::unordered_map<std::string, bool> checkpoints;
    std::unordered_map<std::string, SpatialCell> spatial;
};
