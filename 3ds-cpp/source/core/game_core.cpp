#include "game_core.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

bool TileMap::loadText(const char* path) {
    // Legacy-Textformat laden: zuerst width/height, dann alle Tile-Werte.
    FILE* f = fopen(path, "r");
    if (!f) return false;

    if (fscanf(f, "%d %d", &width, &height) != 2) {
        fclose(f);
        return false;
    }
    tiles.resize(static_cast<size_t>(width * height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int v = 0;
            if (fscanf(f, "%d", &v) != 1) {
                fclose(f);
                return false;
            }
            tiles[static_cast<size_t>(y * width + x)] = static_cast<uint8_t>(v);
        }
    }
    fclose(f);
    buildTransitions(16);
    return true;
}

bool TileMap::loadJson(const char* path) {
    // Sehr einfacher JSON-Reader (ohne externe Bibliothek),
    // ausreichend für das erwartete Kartenformat.
    FILE* f = fopen(path, "r");
    if (!f) return false;
    std::string s;
    char buf[512];
    while (size_t n = fread(buf, 1, sizeof(buf), f)) {
        s.append(buf, n);
    }
    fclose(f);

    auto findInt = [&](const char* key) -> int {
        std::string k = std::string("\"") + key + "\":";
        size_t pos = s.find(k);
        if (pos == std::string::npos) return -1;
        pos += k.size();
        int val = -1;
        sscanf(s.c_str() + pos, "%d", &val);
        return val;
    };

    int w = findInt("width");
    int h = findInt("height");
    if (w <= 0 || h <= 0) return false;
    width = w;
    height = h;
    tiles.assign(static_cast<size_t>(w * h), 0);

    size_t tilesPos = s.find("\"tiles\"");
    if (tilesPos == std::string::npos) return false;
    tilesPos = s.find('[', tilesPos);
    if (tilesPos == std::string::npos) return false;
    size_t i = tilesPos + 1;
    size_t idx = 0;
    while (i < s.size() && idx < tiles.size()) {
        while (i < s.size() && !(s[i] == '-' || (s[i] >= '0' && s[i] <= '9'))) i++;
        if (i >= s.size() || s[i] == ']') break;
        int v = 0;
        int consumed = 0;
        if (sscanf(s.c_str() + i, "%d%n", &v, &consumed) == 1) {
            tiles[idx++] = static_cast<uint8_t>(v);
            i += consumed;
        } else {
            break;
        }
    }
    bool ok = idx == tiles.size();
    if (ok) {
        buildTransitions(16);
        // Zusatz: Items parsen, falls vorhanden
        items.clear();
        size_t itemsPos = s.find("\"items\"");
        if (itemsPos != std::string::npos) {
            size_t arrStart = s.find('[', itemsPos);
            if (arrStart != std::string::npos) {
                size_t i = arrStart + 1;
                while (i < s.size()) {
                    // Suche nächstes Objekt
                    while (i < s.size() && s[i] != '{' && s[i] != ']') ++i;
                    if (i >= s.size() || s[i] == ']') break;
                    size_t objStart = i;
                    int brace = 1;
                    ++i;
                    while (i < s.size() && brace > 0) {
                        if (s[i] == '{') brace++;
                        else if (s[i] == '}') brace--;
                        ++i;
                    }
                    size_t objEnd = i;
                    std::string obj = s.substr(objStart, objEnd - objStart);
                    // Werte extrahieren
                    std::string type;
                    int ix = 0, iy = 0;
                    size_t posType = obj.find("\"type\"");
                    if (posType != std::string::npos) {
                        size_t colon = obj.find(':', posType);
                        if (colon != std::string::npos) {
                            size_t q1 = obj.find('"', colon+1);
                            if (q1 != std::string::npos) {
                                size_t q2 = obj.find('"', q1+1);
                                if (q2 != std::string::npos) {
                                    type = obj.substr(q1+1, q2-q1-1);
                                }
                            }
                        }
                    }
                    size_t posX = obj.find("\"x\"");
                    if (posX != std::string::npos) {
                        std::sscanf(obj.c_str()+posX, "\"x\"%*[^0-9-]%d", &ix);
                    }
                    size_t posY = obj.find("\"y\"");
                    if (posY != std::string::npos) {
                        std::sscanf(obj.c_str()+posY, "\"y\"%*[^0-9-]%d", &iy);
                    }
                    if (!type.empty()) {
                        ItemData id;
                        id.type = type;
                        id.x = ix;
                        id.y = iy;
                        items.push_back(id);
                    }
                }
            }
        }
    }
    return ok;
}

bool TileMap::isSolid(int tx, int ty) const {
    // Außerhalb der Karte zählt als "fest", damit man nicht aus der Welt fällt.
    if (tx < 0 || ty < 0 || tx >= width || ty >= height) return true;
    uint8_t t = tiles[static_cast<size_t>(ty * width + tx)];
    return t == 1 || t == 30 || t == 31 || t == 32 || t == 33;
}

int TileMap::getTile(int tx, int ty) const {
    if (tx < 0 || ty < 0 || tx >= width || ty >= height) return -1;
    return tiles[static_cast<size_t>(ty * width + tx)];
}

void TileMap::buildTransitions(int tileSize) {
    // Tile-ID 3 markiert Raumübergänge; daraus entstehen Trigger-Rechtecke.
    transitions.clear();
    if (width <= 0 || height <= 0) return;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t t = tiles[static_cast<size_t>(y * width + x)];
            if (t == 3) {
                Rect r;
                r.x = x * tileSize;
                r.y = y * tileSize;
                r.w = static_cast<float>(tileSize);
                r.h = static_cast<float>(tileSize);
                transitions.push_back(r);
            }
        }
    }
}

void GameCore::setPlayerStart(float x, float y) {
    // Startpunkt setzen + Bewegungszustand zurücksetzen.
    player.x = x;
    player.y = y;
    spawnX = x;
    spawnY = y;
    dead = false;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.grounded = false;
    player.coyoteTimer = 0.0f;
}

void GameCore::setPlayerPosition(float x, float y) {
    // Direkte Teleport-Position ohne Änderung des Respawn-Startpunkts.
    player.x = x;
    player.y = y;
    dead = false;
    player.vx = 0.0f;
    player.vy = 0.0f;
    player.grounded = false;
    player.coyoteTimer = 0.0f;
}

bool GameCore::consumeDeath() {
    // "Einmal-Event": War der Spieler tot, wird das hier abgeholt und zurückgesetzt.
    bool wasDead = dead;
    dead = false;
    return wasDead;
}

bool GameCore::setPlayerStartToFirstEmpty(float tileSize) {
    // Sucht die erste begehbare Kachel (nicht solid, kein Danger, keine Transition).
    if (map.width <= 0 || map.height <= 0 || map.tiles.empty()) return false;
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            int t = map.getTile(x, y);
            if (!map.isSolid(x, y) && t != 2 && t != 3) {
                setPlayerStart(x * tileSize + 2.0f, y * tileSize + 2.0f);
                return true;
            }
        }
    }
    return false;
}

bool GameCore::loadMapJson(const char* path) {
    return map.loadJson(path);
}

bool GameCore::loadMapText(const char* path) {
    return map.loadText(path);
}

void GameCore::update(const InputState& input, float dt) {
    // Zentrale Physik: Eingaben, Gravitation, Kollisionen (X/Y), Sonderflächen, Tod.
    const float speed = 150.0f;
    const float jumpVel = -420.0f;
    const float gravity = 900.0f;
    const float tileSize = 16.0f;

    // 1) Horizontalinput in Geschwindigkeit umsetzen.
    float inputX = 0.0f;
    if (input.left) inputX -= 1.0f;
    if (input.right) inputX += 1.0f;
    player.vx = inputX * speed;

    // 2) Springen erlaubt bei Bodenkontakt oder kurzer "Coyote-Time".
    if (input.jumpPressed) {
        if (player.grounded || player.coyoteTimer > 0.0f) {
            // normaler erster Sprung
            player.vy = jumpVel;
            player.grounded = false;
            player.coyoteTimer = 0.0f;
            // nach dem Absprung kann ggf. ein zusätzlicher Sprung erfolgen
            player.jumpsRemaining = player.hasDoubleJump ? 1 : 0;
        } else if (player.hasDoubleJump && player.jumpsRemaining > 0) {
            // freier Doppelsprung in der Luft
            player.vy = jumpVel;
            player.jumpsRemaining -= 1;
        }
    }

    // wenn der Spieler wieder den Boden berührt, Reset der Zusatzsprünge
    if (player.grounded) {
        player.jumpsRemaining = player.hasDoubleJump ? 1 : 0;
    }

    // 3) Gravitation beschleunigt den Spieler nach unten.
    player.vy += gravity * dt;

    // 4) X-Kollisionen gegen volle Blöcke inkl. kleiner Step-Up-Hilfe.
    float newX = player.x + player.vx * dt;
    float x0 = std::floor((std::min(player.x, newX)) / tileSize);
    float x1 = std::floor((std::max(player.x + player.w, newX + player.w)) / tileSize);
    float y0 = std::floor(player.y / tileSize);
    float y1 = std::floor((player.y + player.h - 1.0f) / tileSize);
    bool hitX = false;
    for (int ty = static_cast<int>(y0); ty <= static_cast<int>(y1); ++ty) {
        for (int tx = static_cast<int>(x0); tx <= static_cast<int>(x1); ++tx) {
                int t = map.getTile(tx, ty);
                if (t != 1) continue; // only full solid blocks on X
            float tileLeft = tx * tileSize;
            float tileRight = tileLeft + tileSize;
                float tileTop = ty * tileSize;
            if (player.vx > 0.0f && player.x + player.w <= tileLeft && newX + player.w > tileLeft) {
                    float stepHeight = 12.0f;
                    float feetY = player.y + player.h;
                    float penetrationY = feetY - tileTop;
                    if (penetrationY > 0.0f && penetrationY <= stepHeight && player.vy >= 0.0f) {
                        player.y = tileTop - player.h;
                    } else {
                        newX = tileLeft - player.w;
                        hitX = true;
                    }
            } else if (player.vx < 0.0f && player.x >= tileRight && newX < tileRight) {
                    float stepHeight = 12.0f;
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
    if (hitX) player.vx = 0.0f;

    // 5) Y-Kollisionen gegen volle Blöcke und schräge Flächen.
    float newY = player.y + player.vy * dt;
    x0 = std::floor(player.x / tileSize);
    x1 = std::floor((player.x + player.w - 1.0f) / tileSize);
    y0 = std::floor((std::min(player.y, newY)) / tileSize);
    y1 = std::floor((std::max(player.y + player.h, newY + player.h)) / tileSize);
    bool hitY = false;
    bool wasGrounded = player.grounded;
    player.grounded = false;
    for (int ty = static_cast<int>(y0); ty <= static_cast<int>(y1); ++ty) {
        for (int tx = static_cast<int>(x0); tx <= static_cast<int>(x1); ++tx) {
            int t = map.getTile(tx, ty);
            float tileLeft = tx * tileSize;
            float tileTop = ty * tileSize;
            float tileBottom = tileTop + tileSize;

            if (t == 30 || t == 31 || t == 32 || t == 33) {
                float relX = (player.x + player.w * 0.5f) - tileLeft;
                if (relX < 0.0f) relX = 0.0f;
                if (relX > tileSize) relX = tileSize;

                if ((t == 30 || t == 31) && player.vy >= 0.0f) {
                    float targetY = (t == 30) ? (tileTop + (tileSize - relX)) : (tileTop + relX);
                    float penetration = (newY + player.h) - targetY;
                    if (penetration > 0.0f && penetration < 160.0f) {
                        newY = targetY - player.h;
                        hitY = true;
                        player.grounded = true;
                    }
                } else if ((t == 32 || t == 33) && player.vy < 0.0f) {
                    float targetY = (t == 32) ? (tileTop + (tileSize - relX)) : (tileTop + relX);
                    if (newY < targetY) {
                        newY = targetY;
                        hitY = true;
                    }
                }
                continue;
            }

            if (t != 1) continue; // only full solid blocks on Y
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
    if (hitY) player.vy = 0.0f;

    if (player.grounded) {
        // Spieler berührt den Boden – Zusatzsprung zurücksetzen
        player.jumpsRemaining = player.hasDoubleJump ? 1 : 0;
        player.coyoteTimer = 0.1f;
    } else if (wasGrounded && !player.grounded && player.vy >= 0.0f) {
        // 6) Slope-Snap: verhindert, dass man beim Laufen über Schräge kurz "schwebt".
        float centerX = player.x + player.w * 0.5f;
        int baseTx = static_cast<int>(std::floor(centerX / tileSize));
        int ty = static_cast<int>(std::floor((player.y + player.h + 12.0f) / tileSize));
        for (int tx = baseTx - 1; tx <= baseTx + 1; ++tx) {
            int t = map.getTile(tx, ty);
            if (t != 30 && t != 31) continue;
            float tileLeft = tx * tileSize;
            float tileTop = ty * tileSize;
            float relX = centerX - tileLeft;
            if (relX < 0.0f) relX = 0.0f;
            if (relX > tileSize) relX = tileSize;
            float targetY = (t == 30) ? (tileTop + (tileSize - relX)) : (tileTop + relX);
            float diff = targetY - (player.y + player.h);
            if (diff >= -2.0f && diff <= 12.0f) {
                player.y = targetY - player.h;
                player.vy = 0.0f;
                player.grounded = true;
                break;
            }
        }
        player.coyoteTimer = 0.1f;
    } else {
        player.coyoteTimer -= dt;
        if (player.coyoteTimer < 0.0f) player.coyoteTimer = 0.0f;
    }

    // 7) Gefahrenkacheln (Tile-ID 2) markieren den Spieler als tot.
    int tx0 = static_cast<int>(std::floor(player.x / tileSize));
    int tx1 = static_cast<int>(std::floor((player.x + player.w - 1.0f) / tileSize));
    int ty0 = static_cast<int>(std::floor(player.y / tileSize));
    int ty1 = static_cast<int>(std::floor((player.y + player.h - 1.0f) / tileSize));
    for (int ty = ty0; ty <= ty1; ++ty) {
        for (int tx = tx0; tx <= tx1; ++tx) {
            if (map.getTile(tx, ty) == 2) {
                dead = true;
                return;
            }
        }
    }
}
