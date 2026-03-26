#pragma once

#include "../../core/game_core.h"

namespace playerlogic {

// Prueft, ob der Spieler in einer Gefahrenkachel (Tile-ID 2) steht.
bool detectDangerCollision(const Player& player, const TileMap& map, float tileSize);

} // namespace playerlogic
