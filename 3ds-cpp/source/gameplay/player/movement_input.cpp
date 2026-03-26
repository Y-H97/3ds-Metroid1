#include "movement_input.h"

namespace playerlogic {

// Uebersetzt den Links-/Rechts-Input in eine horizontale Zielgeschwindigkeit.
// Das Modul ist bewusst klein gehalten, damit Eingabelogik getrennt von
// Sprung- und Kollisionslogik angepasst werden kann.
void applyHorizontalInput(Player& player, const InputState& input, float speed) {
    float inputX = 0.0f;
    if (input.left) inputX -= 1.0f;
    if (input.right) inputX += 1.0f;
    player.vx = inputX * speed;
}

} // namespace playerlogic
