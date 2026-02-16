#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct WorldCell {
    // Zellkoordinate im Welt-Grid + zugehöriger Levelname.
    int x = 0;
    int y = 0;
    std::string level;
};

struct SpatialCell {
    // Eine sichtbare Grid-Zelle zeigt auf ein Level und dessen Ursprungszelle.
    std::string level;
    int originX = 0;
    int originY = 0;
};

class WorldMap {
public:
    // Lädt world.json mit Zellen und Checkpoint-Markierungen.
    bool loadWorldJson(const char* path);
    // Baut aus den Levelgrößen eine aufgelöste Spatial-Map für Bildschirmsegmente.
    bool buildSpatialMap(const char* mapsRoot, int tileSize, int screenW, int screenH);

    const std::vector<WorldCell>& getCells() const { return cells; }
    // Sucht die Spatial-Information zu einer Grid-Zelle.
    const SpatialCell* getCell(int gridX, int gridY) const;
    // Liefert Min/Max-Grenzen aller bekannten Spatial-Zellen.
    bool getSpatialBounds(int& minX, int& minY, int& maxX, int& maxY) const;

    // Prüft, ob die Grid-Zelle als Checkpoint definiert ist.
    bool isCheckpoint(int gridX, int gridY) const;

private:
    std::vector<WorldCell> cells;
    std::unordered_map<std::string, bool> checkpoints;
    std::unordered_map<std::string, SpatialCell> spatial;
};
