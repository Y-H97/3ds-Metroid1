#pragma once

#include "../../core/game_core.h"

namespace playerlogic {

// Uebersetzt den links/rechts-Input in eine horizontale Spieler-Geschwindigkeit.
void applyHorizontalInput(Player& player, const InputState& input, float speed);

} // namespace playerlogic
