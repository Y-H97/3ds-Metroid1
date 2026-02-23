#include "double_jump.h"
#include "../../gameplay_scene.h"

namespace items {
namespace double_jump {

void onCollect(GameplayScene& scene) {
    // Aktiviert die doppelte Sprungfähigkeit im GameCore/Player.
    auto& corePlayer = scene.core.getPlayer();
    // Wir setzen ein Flag direkt im Player, das vom GameCore-Update verwendet wird.
    // Alternativ könnte das Flag aus den aktiven Items berechnet werden, aber
    // das ist hier ausreichend.
    // Direct access ist hier möglich, da GameplayScene ein Freund ist oder wir
    // stattdessen eine Methode bereitstellen (wir nutzen Direkter Zugriff).
    corePlayer.hasDoubleJump = true;

    // Zeige eine Textbenachrichtigung an.
    scene.pickupMessage = "Doppelsprung erhalten";
    scene.pickupMessageTimer = 2.0f;
}

} // namespace double_jump
} // namespace items
