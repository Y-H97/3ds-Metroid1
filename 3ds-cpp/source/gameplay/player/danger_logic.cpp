#include "danger_logic.h"

#include <cmath>

namespace playerlogic {

// Prueft, ob die aktuelle Spielerbox eine Gefahrenkachel beruehrt.
// Tile-ID 2 steht hier fuer schadende oder toedliche Felder.
bool detectDangerCollision(const Player& player, const TileMap& map, float tileSize) {
    int tx0 = static_cast<int>(std::floor(player.x / tileSize));
    int tx1 = static_cast<int>(std::floor((player.x + player.w - 1.0f) / tileSize));
    int ty0 = static_cast<int>(std::floor(player.y / tileSize));
    int ty1 = static_cast<int>(std::floor((player.y + player.h - 1.0f) / tileSize));

    for (int ty = ty0; ty <= ty1; ++ty) {
        for (int tx = tx0; tx <= tx1; ++tx) {
            // Sobald eine einzige Gefahrenkachel getroffen wird, koennen wir
            // sofort abbrechen. Mehr Details braucht die Aufruferseite nicht.
            if (map.getTile(tx, ty) == 2) {
                return true;
            }
        }
    }

    return false;
}

} // namespace playerlogic
