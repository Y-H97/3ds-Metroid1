#pragma once

#include <cstdint>
#include <vector>

struct InputState {
    // Bewegungs- und Sprungwünsche für ein einzelnes Update-Frame.
    bool left = false;
    bool right = false;
    bool jump = false;
    bool jumpPressed = false;
};

struct Player {
    // Physik-Zustand des Spielers in Pixel-Koordinaten.
    float x = 40.0f;
    float y = 40.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float w = 20.0f;
    float h = 30.0f;
    bool grounded = false;
    float coyoteTimer = 0.0f;

    // Zusatz für Doppelsprung: besitzt der Spieler das Upgrade?
    bool hasDoubleJump = false;
    // Zählt verbleibende Sprünge (0 = nur normaler Boden-Sprung möglich).
    int jumpsRemaining = 0;
};

struct Rect {
    // Einfache Rechteck-Struktur, z. B. für Trigger/Transitions.
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

class TileMap {
public:
    // Kacheln horizontal/vertikal.
    int width = 0;
    int height = 0;
    // Lineares Feld aller Tiles (Index: y * width + x).
    std::vector<uint8_t> tiles;

    // Items, die beim Export aus dem Editor mitgeliefert werden.
    struct ItemData {
        std::string type;
        int x = 0;
        int y = 0;
    };
    std::vector<ItemData> items;

    // Lädt eine alte Text-Kartenrepräsentation.
    bool loadText(const char* path);
    // Lädt das aktuelle JSON-Kartenformat.
    bool loadJson(const char* path);
    // Prüft, ob eine Kachel als "fest" gilt (Kollision).
    bool isSolid(int tx, int ty) const;
    // Liefert die Tile-ID an Gitterposition tx/ty.
    int getTile(int tx, int ty) const;
    // Gibt alle Übergangs-Rechtecke (Tile-ID 3) zurück.
    const std::vector<Rect>& getTransitions() const { return transitions; }

    // Sucht in der Tilemap alle Übergangs-Tiles und baut Trigger-Rechtecke auf.
    void buildTransitions(int tileSize);

private:
    std::vector<Rect> transitions;
};

class GameCore {
public:
    // Karte laden (JSON / Legacy-Text).
    bool loadMapJson(const char* path);
    bool loadMapText(const char* path);

    // Führt ein Physik- und Kollisions-Update aus.
    void update(const InputState& input, float dt);

    const Player& getPlayer() const { return player; }
    Player& getPlayer() { return player; }

    const TileMap& getMap() const { return map; }
    TileMap& getMap() { return map; }

    // Setzt den Respawn-Startpunkt (Checkpoint-ähnlich).
    void setPlayerStart(float x, float y);
    // Sucht die erste begehbare Kachel und setzt dort den Startpunkt.
    bool setPlayerStartToFirstEmpty(float tileSize);
    // Versetzt den Spieler direkt (ohne Respawnpunkt zu ändern).
    void setPlayerPosition(float x, float y);
    // Liefert einmalig den "tot"-Zustand und setzt ihn zurück.
    bool consumeDeath();

private:
    TileMap map;
    Player player;
    float spawnX = 40.0f;
    float spawnY = 40.0f;
    bool dead = false;
};
