#include "player_logic.h"

#include "collision_logic.h"
#include "danger_logic.h"
#include "jump_logic.h"
#include "movement_input.h"

namespace playerlogic {

// Fuehrt einen kompletten Bewegungs-Frame fuer den Spieler aus.
// Die Reihenfolge ist wichtig: erst Eingabe und Sprungstatus, dann Gravitation,
// danach Kollisionen und abschliessend die Gefahrenpruefung.
bool updatePlayerMovement(Player& player, const InputState& input, const TileMap& map, float dt) {
    PlayerMovementConfig movementCfg;
    JumpConfig jumpCfg;

    // Die horizontale Bewegung wird direkt aus dem aktuellen Eingabestatus berechnet.
    applyHorizontalInput(player, input, movementCfg.speed);

    // Sprungeingaben muessen vor der Kollision ausgewertet werden, damit zum
    // Beispiel Coyote-Time und variabler Sprung sauber funktionieren.
    applyJumpPress(player, input, jumpCfg);
    refreshGroundJumpState(player);
    applyVariableJumpCut(player, input, jumpCfg);

    // Gravitation wirkt in jedem Frame auf die vertikale Geschwindigkeit.
    player.vy += movementCfg.gravity * dt;

    // Zuerst horizontal loesen, damit Wandkontakte die spaetere Vertikalkorrektur
    // nicht unnoetig verfaelschen.
    resolveHorizontalCollisions(player, map, dt, movementCfg.tileSize, movementCfg.stepHeight);

    bool wasGrounded = player.grounded;

    // Danach folgt die vertikale Loesung mit Boden, Decke und Schraegflaechen.
    resolveVerticalCollisions(player, map, dt, movementCfg.tileSize);

    if (player.grounded) {
        // Beim sicheren Bodenkontakt werden Sprungreserven und Status neu gesetzt.
        refreshGroundJumpState(player);
    }

    // Die Coyote-Time nutzt den alten und den neuen Bodenzustand, um kurze
    // Fehlertoleranz nach dem Verlassen einer Kante zu erlauben.
    updateCoyoteTimer(player, wasGrounded, map, dt, movementCfg.tileSize, movementCfg.coyoteDuration);

    // Der Rueckgabewert signalisiert der Gameplay-Schicht, ob ein Respawn oder
    // Schaden durch Gefahrenkacheln ausgeloest werden muss.
    return detectDangerCollision(player, map, movementCfg.tileSize);
}

} // namespace playerlogic
