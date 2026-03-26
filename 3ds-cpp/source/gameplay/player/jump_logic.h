#pragma once

#include "../../core/game_core.h"

namespace playerlogic {

// Konfigurationswerte, die nur die Sprungmechanik steuern.
struct JumpConfig {
    float jumpVelocity = -420.0f;
    float jumpCutVelocity = -190.0f;
};

// Verarbeitet Sprung-Input (Bodensprung, Coyote-Time, Doppelsprung).
void applyJumpPress(Player& player, const InputState& input, const JumpConfig& cfg);

// Kuerzt den Aufstieg, wenn die Sprungtaste frueh losgelassen wird.
void applyVariableJumpCut(Player& player, const InputState& input, const JumpConfig& cfg);

// Setzt beim Bodenkontakt die verfuegbaren Zusatzspruenge konsistent zurueck.
void refreshGroundJumpState(Player& player);

// Aktualisiert den Coyote-Timer nach der Kollisionsphase.
void updateCoyoteTimer(Player& player, bool wasGrounded, const TileMap& map, float dt, float tileSize, float coyoteDuration);

} // namespace playerlogic
