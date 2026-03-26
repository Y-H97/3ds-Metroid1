#pragma once

#include "../../core/game_core.h"

namespace playerlogic {

// Gemeinsame Bewegungs-/Physikparameter fuer das Spieler-Update.
struct PlayerMovementConfig {
    float speed = 150.0f;
    float gravity = 900.0f;
    float tileSize = 16.0f;
    float coyoteDuration = 0.1f;
    float stepHeight = 12.0f;
};

// Fuehrt ein komplettes Frame-Update fuer den Spieler aus.
// Rueckgabe: true, wenn der Spieler in einer Gefahrenkachel steht (Tod).
bool updatePlayerMovement(Player& player, const InputState& input, const TileMap& map, float dt);

} // namespace playerlogic
