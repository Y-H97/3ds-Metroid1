#include "jump_logic.h"

#include <cmath>

namespace {

// Hilfsfunktion fuer schraege Bodenkacheln: berechnet die erwartete Y-Hoehe am X-Punkt.
float slopeSurfaceY(int tileId, float tileTop, float tileSize, float relX) {
    if (tileId == 30) {
        return tileTop + (tileSize - relX);
    }
    return tileTop + relX;
}

} // namespace

namespace playerlogic {

void applyJumpPress(Player& player, const InputState& input, const JumpConfig& cfg) {
    if (!input.jumpPressed) {
        return;
    }

    if (player.grounded || player.coyoteTimer > 0.0f) {
        // Normaler Startsprung vom Boden (oder waehrend Coyote-Time).
        player.vy = cfg.jumpVelocity;
        player.grounded = false;
        player.coyoteTimer = 0.0f;
        player.jumpsRemaining = player.hasDoubleJump ? 1 : 0;
        return;
    }

    if (player.hasDoubleJump && player.jumpsRemaining > 0) {
        // Freier Luftsprung, solange noch ein Zusatzsprung verfuegbar ist.
        player.vy = cfg.jumpVelocity;
        player.jumpsRemaining -= 1;
    }
}

void applyVariableJumpCut(Player& player, const InputState& input, const JumpConfig& cfg) {
    // Fruehes Loslassen begrenzt nur den aufwaerts gerichteten Anteil des Sprungs.
    if (!input.jump && player.vy < cfg.jumpCutVelocity) {
        player.vy = cfg.jumpCutVelocity;
    }
}

void refreshGroundJumpState(Player& player) {
    if (player.grounded) {
        player.jumpsRemaining = player.hasDoubleJump ? 1 : 0;
    }
}

void updateCoyoteTimer(Player& player, bool wasGrounded, const TileMap& map, float dt, float tileSize, float coyoteDuration) {
    if (player.grounded) {
        player.coyoteTimer = coyoteDuration;
        return;
    }

    if (wasGrounded && player.vy >= 0.0f) {
        // Slope-Snap: verhindert beim Lauf ueber Schraegkacheln ein kurzes "Abheben".
        float centerX = player.x + player.w * 0.5f;
        int baseTx = static_cast<int>(std::floor(centerX / tileSize));
        int ty = static_cast<int>(std::floor((player.y + player.h + 12.0f) / tileSize));

        for (int tx = baseTx - 1; tx <= baseTx + 1; ++tx) {
            int tile = map.getTile(tx, ty);
            if (tile != 30 && tile != 31) {
                continue;
            }

            float tileLeft = tx * tileSize;
            float relX = centerX - tileLeft;
            if (relX < 0.0f) relX = 0.0f;
            if (relX > tileSize) relX = tileSize;

            float targetY = slopeSurfaceY(tile, ty * tileSize, tileSize, relX);
            float diff = targetY - (player.y + player.h);
            if (diff >= -2.0f && diff <= 12.0f) {
                player.y = targetY - player.h;
                player.vy = 0.0f;
                player.grounded = true;
                break;
            }
        }

        player.coyoteTimer = coyoteDuration;
        return;
    }

    player.coyoteTimer -= dt;
    if (player.coyoteTimer < 0.0f) {
        player.coyoteTimer = 0.0f;
    }
}

} // namespace playerlogic
