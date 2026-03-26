#include "double_jump.h"
#include "../../gameplay_scene.h"

namespace items {
namespace double_jump {

void onCollect(GameplayScene& scene) {
    // Item-Effekt und UI-Hinweis laufen ueber die oeffentlichen Szenenmethoden.
    scene.grantDoubleJump();
    scene.showPickupMessage("Doppelsprung erhalten", 2.0f);
}

} // namespace double_jump
} // namespace items
