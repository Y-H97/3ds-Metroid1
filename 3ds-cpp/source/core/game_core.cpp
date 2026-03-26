#include "game_core.h"
#include "../gameplay/player/player_logic.h"

bool TileMap::isSolid(int tx, int ty) const {
    // Außerhalb der Karte zählt als "fest", damit man nicht aus der Welt fällt.
    if (tx < 0 || ty < 0 || tx >= width || ty >= height) return true;
    uint8_t t = tiles[static_cast<size_t>(ty * width + tx)];
    return t == 1 || t == 30 || t == 31 || t == 32 || t == 33;
}

int TileMap::getTile(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= width || ty >= height) return -1;
    return tiles[static_cast<size_t>(ty * width + tx)];
}

void TileMap::buildTransitions(int tileSize) {
    // Tile-ID 3 markiert Raumübergänge; daraus entstehen Trigger-Rechtecke.
    transitions.clear();
    if (width <= 0 || height <= 0) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t t = tiles[static_cast<size_t>(y * width + x)];
            if (t == 3) {
                Rect r;
                r.x = x * tileSize;
                r.y = y * tileSize;
                r.w = static_cast<float>(tileSize);
                r.h = static_cast<float>(tileSize);
                transitions.push_back(r);
            }
        }
    }
}

void GameCore::setPlayerStart(float x, float y) {
    // Startpunkt setzen + Bewegungszustand zurücksetzen.
    player.x = x;
    player.y = y;
    spawnX = x;
    spawnY = y;
    dead = false;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.grounded = false;
    player.coyoteTimer = 0.0f;
}

void GameCore::setPlayerPosition(float x, float y) {
    // Direkte Teleport-Position ohne Änderung des Respawn-Startpunkts.
    player.x = x;
    player.y = y;
    dead = false;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.grounded = false;
    player.coyoteTimer = 0.0f;
}

bool GameCore::consumeDeath() {
    // "Einmal-Event": War der Spieler tot, wird das hier abgeholt und zurückgesetzt.
    bool wasDead = dead;
    dead = false;
    return wasDead;
}

bool GameCore::setPlayerStartToFirstEmpty(float tileSize) {
    // Sucht die erste begehbare Kachel (nicht solid, kein Danger, keine Transition).
    if (map.width <= 0 || map.height <= 0 || map.tiles.empty()) return false;
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            int t = map.getTile(x, y);
            if (!map.isSolid(x, y) && t != 2 && t != 3) {
                setPlayerStart(x * tileSize + 2.0f, y * tileSize + 2.0f);
                return true;
            }
        }
    }
    return false;
}

bool GameCore::loadMapJson(const char* path) {
    return map.loadJson(path);
}

bool GameCore::loadMapText(const char* path) {
    return map.loadText(path);
}

void GameCore::update(const InputState& input, float dt) {
    // Der eigentliche Bewegungs- und Sprungcode lebt jetzt in einem eigenen Modul.
    // GameCore bleibt dadurch als orchestrierende Fassade schlank.
    if (playerlogic::updatePlayerMovement(player, input, map, dt)) {
        dead = true;
    }
}
