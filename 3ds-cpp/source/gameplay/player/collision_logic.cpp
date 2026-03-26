#include "collision_logic.h"

#include <algorithm>
#include <cmath>

namespace playerlogic {

// Bewegt den Spieler horizontal und stoppt ihn an massiven Kacheln.
// Die Funktion erlaubt kleine Stufen, damit der Spieler bei flachen Kanten
// nicht haengen bleibt, sondern sauber aufsteigen kann.
void resolveHorizontalCollisions(Player& player, const TileMap& map, float dt, float tileSize, float stepHeight) {
    float newX = player.x + player.vx * dt;
    float x0 = std::floor((std::min(player.x, newX)) / tileSize);
    float x1 = std::floor((std::max(player.x + player.w, newX + player.w)) / tileSize);
    float y0 = std::floor(player.y / tileSize);
    float y1 = std::floor((player.y + player.h - 1.0f) / tileSize);
    bool hitX = false;

    for (int ty = static_cast<int>(y0); ty <= static_cast<int>(y1); ++ty) {
        for (int tx = static_cast<int>(x0); tx <= static_cast<int>(x1); ++tx) {
            int tile = map.getTile(tx, ty);
            if (tile != 1) {
                continue;
            }

            float tileLeft = tx * tileSize;
            float tileRight = tileLeft + tileSize;
            float tileTop = ty * tileSize;

            // Beim Laufen nach rechts pruefen wir, ob die Spielerbox die linke
            // Kante der Kachel kreuzt. Kleine Hoehenunterschiede behandeln wir
            // als Stufe statt als harte Wand.
            if (player.vx > 0.0f && player.x + player.w <= tileLeft && newX + player.w > tileLeft) {
                float feetY = player.y + player.h;
                float penetrationY = feetY - tileTop;
                if (penetrationY > 0.0f && penetrationY <= stepHeight && player.vy >= 0.0f) {
                    player.y = tileTop - player.h;
                } else {
                    newX = tileLeft - player.w;
                    hitX = true;
                }
            // Dasselbe Prinzip gilt gespiegelt fuer die Bewegung nach links.
            } else if (player.vx < 0.0f && player.x >= tileRight && newX < tileRight) {
                float feetY = player.y + player.h;
                float penetrationY = feetY - tileTop;
                if (penetrationY > 0.0f && penetrationY <= stepHeight && player.vy >= 0.0f) {
                    player.y = tileTop - player.h;
                } else {
                    newX = tileRight;
                    hitX = true;
                }
            }
        }
    }

    player.x = newX;
    if (hitX) {
        player.vx = 0.0f;
    }
}

// Bewegt den Spieler vertikal, behandelt Boden-/Deckenkontakte und wertet
// zusaetzlich Schraegkacheln aus. Diese Funktion entscheidet ausserdem,
// ob der Spieler aktuell als "grounded" gilt.
void resolveVerticalCollisions(Player& player, const TileMap& map, float dt, float tileSize) {
    float newY = player.y + player.vy * dt;
    bool hitY = false;

    {
        // Die Welt hat harte Ober- und Untergrenzen, damit der Spieler nie
        // ausserhalb der Map landet.
        float maxYpos = map.height * tileSize - player.h;
        if (newY > maxYpos) {
            newY = maxYpos;
            hitY = true;
            player.grounded = true;
            player.vy = 0.0f;
        }
        if (newY < 0.0f) {
            newY = 0.0f;
            player.vy = 0.0f;
        }
    }

    float x0 = std::floor(player.x / tileSize);
    float x1 = std::floor((player.x + player.w - 1.0f) / tileSize);
    float y0 = std::floor((std::min(player.y, newY)) / tileSize);
    float y1 = std::floor((std::max(player.y + player.h, newY + player.h)) / tileSize);
    player.grounded = false;

    for (int ty = static_cast<int>(y0); ty <= static_cast<int>(y1); ++ty) {
        for (int tx = static_cast<int>(x0); tx <= static_cast<int>(x1); ++tx) {
            int tile = map.getTile(tx, ty);
            float tileLeft = tx * tileSize;
            float tileTop = ty * tileSize;
            float tileBottom = tileTop + tileSize;

            // Die Tile-IDs 30-33 repraesentieren Schraegflaechen. Statt einer
            // rechteckigen Kollision berechnen wir die Zielhoehe innerhalb der
            // Kachel anhand der horizontalen Spielerposition.
            if (tile == 30 || tile == 31 || tile == 32 || tile == 33) {
                float relX = (player.x + player.w * 0.5f) - tileLeft;
                if (relX < 0.0f) relX = 0.0f;
                if (relX > tileSize) relX = tileSize;

                // Untere Schraegkacheln tragen den Spieler beim Fallen.
                if ((tile == 30 || tile == 31) && player.vy >= 0.0f) {
                    float targetY = (tile == 30) ? (tileTop + (tileSize - relX)) : (tileTop + relX);
                    float penetration = (newY + player.h) - targetY;
                    if (penetration > 0.0f && penetration < 160.0f) {
                        newY = targetY - player.h;
                        hitY = true;
                        player.grounded = true;
                    }
                // Obere Schraegkacheln blockieren den Spieler beim Sprung nach oben.
                } else if ((tile == 32 || tile == 33) && player.vy < 0.0f) {
                    float targetY = (tile == 32) ? (tileTop + (tileSize - relX)) : (tileTop + relX);
                    if (newY < targetY) {
                        newY = targetY;
                        hitY = true;
                    }
                }
                continue;
            }

            if (tile != 1) {
                continue;
            }

            // Normale Vollkacheln behandeln wir klassisch als Boden oder Decke.
            if (player.vy > 0.0f && player.y + player.h <= tileTop && newY + player.h > tileTop) {
                newY = tileTop - player.h;
                hitY = true;
                player.grounded = true;
            } else if (player.vy < 0.0f && player.y >= tileBottom && newY < tileBottom) {
                newY = tileBottom;
                hitY = true;
            }
        }
    }

    player.y = newY;
    if (hitY) {
        player.vy = 0.0f;
    }
}

} // namespace playerlogic
