#include "movement_input.h"

namespace playerlogic {

void applyHorizontalInput(Player& player, const InputState& input, float speed) {
    float inputX = 0.0f;
    if (input.left) inputX -= 1.0f;
    if (input.right) inputX += 1.0f;
    player.vx = inputX * speed;
}

} // namespace playerlogic
