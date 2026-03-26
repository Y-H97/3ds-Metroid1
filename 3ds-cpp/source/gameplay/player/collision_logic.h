#pragma once

#include "../../core/game_core.h"

namespace playerlogic {

// Loest seitliche Kollisionen auf und beruecksichtigt ein kleines Step-Up-Fenster.
void resolveHorizontalCollisions(Player& player, const TileMap& map, float dt, float tileSize, float stepHeight);

// Loest vertikale Kollisionen inkl. Schraegkacheln und Grounding-Zustand.
void resolveVerticalCollisions(Player& player, const TileMap& map, float dt, float tileSize);

} // namespace playerlogic
