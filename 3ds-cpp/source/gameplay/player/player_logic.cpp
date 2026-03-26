#include "player_logic.h"

#include "collision_logic.h"
#include "danger_logic.h"
#include "jump_logic.h"
#include "movement_input.h"

namespace playerlogic {

bool updatePlayerMovement(Player& player, const InputState& input, const TileMap& map, float dt) {
    PlayerMovementConfig movementCfg;
    JumpConfig jumpCfg;

    applyHorizontalInput(player, input, movementCfg.speed);

    applyJumpPress(player, input, jumpCfg);
    refreshGroundJumpState(player);
    applyVariableJumpCut(player, input, jumpCfg);

    player.vy += movementCfg.gravity * dt;

    resolveHorizontalCollisions(player, map, dt, movementCfg.tileSize, movementCfg.stepHeight);

    bool wasGrounded = player.grounded;
    resolveVerticalCollisions(player, map, dt, movementCfg.tileSize);

    if (player.grounded) {
        refreshGroundJumpState(player);
    }

    updateCoyoteTimer(player, wasGrounded, map, dt, movementCfg.tileSize, movementCfg.coyoteDuration);

    return detectDangerCollision(player, map, movementCfg.tileSize);
}

} // namespace playerlogic
