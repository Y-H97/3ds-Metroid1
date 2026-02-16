#include "world_map.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "game_core.h"

static std::string makeKey(int x, int y) {
    // Einheitlicher Schlüssel für Hash-Maps: "x,y".
    return std::to_string(x) + "," + std::to_string(y);
}

static int findIntInObject(const std::string& obj, const char* key, bool& ok) {
    ok = false;
    std::string k = std::string("\"") + key + "\"";
    size_t pos = obj.find(k);
    if (pos == std::string::npos) return 0;
    pos = obj.find(':', pos);
    if (pos == std::string::npos) return 0;
    int val = 0;
    if (std::sscanf(obj.c_str() + pos + 1, " %d", &val) == 1) {
        ok = true;
        return val;
    }
    return 0;
}

bool WorldMap::loadWorldJson(const char* path) {
    // Liest world.json und extrahiert:
    // - cells: welche Level liegen auf welchen Weltkoordinaten
    // - checkpoints: welche Weltzellen als Checkpoint zählen
    cells.clear();
    checkpoints.clear();

    FILE* f = fopen(path, "r");
    if (!f) return false;
    std::string s;
    char buf[512];
    while (size_t n = fread(buf, 1, sizeof(buf), f)) {
        s.append(buf, n);
    }
    fclose(f);

    size_t cellsPos = s.find("\"cells\"");
    if (cellsPos == std::string::npos) return false;
    cellsPos = s.find('[', cellsPos);
    if (cellsPos == std::string::npos) return false;

    size_t i = cellsPos + 1;
    while (i < s.size()) {
        size_t objStart = s.find('{', i);
        if (objStart == std::string::npos) break;
        size_t objEnd = s.find('}', objStart);
        if (objEnd == std::string::npos) break;
        std::string obj = s.substr(objStart, objEnd - objStart + 1);

        int x = 0;
        int y = 0;
        char levelBuf[256] = {0};
        bool okX = false;
        bool okY = false;
        x = findIntInObject(obj, "x", okX);
        y = findIntInObject(obj, "y", okY);
        size_t levelPos = obj.find("\"level\"");
        bool okLevel = false;
        if (levelPos != std::string::npos) {
            levelPos = obj.find('"', levelPos + 7);
            if (levelPos != std::string::npos) {
                size_t end = obj.find('"', levelPos + 1);
                if (end != std::string::npos) {
                    std::string lvl = obj.substr(levelPos + 1, end - levelPos - 1);
                    std::snprintf(levelBuf, sizeof(levelBuf), "%s", lvl.c_str());
                    okLevel = true;
                }
            }
        }
        if (okX && okY && okLevel) {
            WorldCell cell;
            cell.x = x;
            cell.y = y;
            cell.level = levelBuf;
            cells.push_back(cell);
        }

        i = objEnd + 1;
    }

    size_t checkpointsPos = s.find("\"checkpoints\"");
    if (checkpointsPos != std::string::npos) {
        checkpointsPos = s.find('{', checkpointsPos);
        size_t checkpointsEnd = s.find('}', checkpointsPos);
        if (checkpointsPos != std::string::npos && checkpointsEnd != std::string::npos) {
            std::string obj = s.substr(checkpointsPos + 1, checkpointsEnd - checkpointsPos - 1);
            size_t p = 0;
            while (p < obj.size()) {
                size_t keyStart = obj.find('"', p);
                if (keyStart == std::string::npos) break;
                size_t keyEnd = obj.find('"', keyStart + 1);
                if (keyEnd == std::string::npos) break;
                std::string key = obj.substr(keyStart + 1, keyEnd - keyStart - 1);
                checkpoints[key] = true;
                p = keyEnd + 1;
            }
        }
    }

    return !cells.empty();
}

bool WorldMap::buildSpatialMap(const char* mapsRoot, int tileSize, int screenW, int screenH) {
    // Große Level können mehrere Bildschirmsegmente belegen.
    // Diese Funktion füllt jedes Segment als eigene Grid-Zelle in "spatial".
    spatial.clear();
    if (cells.empty()) return false;

    for (const auto& cell : cells) {
        std::string levelPath = std::string(mapsRoot) + "/" + cell.level + ".json";
        TileMap tmp;
        if (!tmp.loadJson(levelPath.c_str())) {
            continue;
        }
        int wPx = tmp.width * tileSize;
        int hPx = tmp.height * tileSize;
        int screensW = static_cast<int>(std::ceil(static_cast<float>(wPx) / screenW));
        int screensH = static_cast<int>(std::ceil(static_cast<float>(hPx) / screenH));
        if (screensW < 1) screensW = 1;
        if (screensH < 1) screensH = 1;

        for (int dy = 0; dy < screensH; ++dy) {
            for (int dx = 0; dx < screensW; ++dx) {
                int gx = cell.x + dx;
                int gy = cell.y + dy;
                SpatialCell sc;
                sc.level = cell.level;
                sc.originX = cell.x;
                sc.originY = cell.y;
                spatial[makeKey(gx, gy)] = sc;
            }
        }
    }

    return !spatial.empty();
}

const SpatialCell* WorldMap::getCell(int gridX, int gridY) const {
    // Direkter Lookup einer Weltzelle.
    auto it = spatial.find(makeKey(gridX, gridY));
    if (it == spatial.end()) return nullptr;
    return &it->second;
}

bool WorldMap::getSpatialBounds(int& minX, int& minY, int& maxX, int& maxY) const {
    // Berechnet das umschließende Rechteck aller Weltzellen.
    if (spatial.empty()) return false;

    bool first = true;
    for (const auto& it : spatial) {
        int x = 0;
        int y = 0;
        if (std::sscanf(it.first.c_str(), "%d,%d", &x, &y) != 2) continue;
        if (first) {
            minX = maxX = x;
            minY = maxY = y;
            first = false;
            continue;
        }
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
    return !first;
}

bool WorldMap::isCheckpoint(int gridX, int gridY) const {
    // Checkpoint-Status stammt aus world.json -> checkpoints.
    auto it = checkpoints.find(makeKey(gridX, gridY));
    return it != checkpoints.end();
}
