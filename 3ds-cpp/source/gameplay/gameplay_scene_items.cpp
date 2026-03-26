#include "gameplay_scene.h"

#include "items.h"

void GameplayScene::refreshMapItems() {
    // Baut aus den exportierten Map-Items die Laufzeitliste fuer die aktuelle Karte auf.
    mapItems.clear();
    const TileMap& map = core.getMap();
    const float tileSize = 16.0f;

    for (const auto& data : map.items) {
        // Bereits dauerhaft eingesammelte Exemplare werden nicht erneut gespawnt.
        if (data.type == items::double_jump::id() && (collectedItems & ITEM_DOUBLE_JUMP)) {
            continue;
        }

        MapItem item;
        item.type = data.type;
        item.x = data.x * tileSize;
        item.y = data.y * tileSize;
        item.collected = false;
        mapItems.push_back(item);
    }
}

void GameplayScene::markItemsOverlappingPlayerAsCollected() {
    // Verhindert, dass ein neues Spiel sofort ein Start-Item einsammelt.
    const Player& player = core.getPlayer();
    const float itemSize = 16.0f;

    for (auto& item : mapItems) {
        if (item.collected) {
            continue;
        }

        if (player.x < item.x + itemSize && player.x + player.w > item.x &&
            player.y < item.y + itemSize && player.y + player.h > item.y) {
            item.collected = true;
        }
    }
}

void GameplayScene::setDoubleJumpEnabled(bool enabled) {
    // Aktiviert/deaktiviert den Runtime-Effekt zentral an genau einer Stelle.
    core.getPlayer().hasDoubleJump = enabled;
    if (!enabled) {
        core.getPlayer().jumpsRemaining = 0;
    }
}

void GameplayScene::grantDoubleJump() {
    setDoubleJumpEnabled(true);
}

bool GameplayScene::playerHasDoubleJump() const {
    return core.getPlayer().hasDoubleJump;
}

void GameplayScene::showPickupMessage(const char* message, float durationSeconds) {
    // UI-Hinweis fuer eingesammelte Items zentral setzen.
    pickupMessage = message ? message : "";
    pickupMessageTimer = durationSeconds;
}

void GameplayScene::collectOverlappingItems() {
    // Prueft pro Frame, ob der Spieler ein noch aktives Map-Item beruehrt.
    const Player& player = core.getPlayer();
    const float itemSize = 16.0f;

    for (auto& item : mapItems) {
        if (item.collected) {
            continue;
        }

        bool overlaps = player.x < item.x + itemSize && player.x + player.w > item.x &&
            player.y < item.y + itemSize && player.y + player.h > item.y;
        if (!overlaps) {
            continue;
        }

        item.collected = true;
        if (item.type == items::double_jump::id()) {
            collectedItems |= ITEM_DOUBLE_JUMP;
            activeItems |= ITEM_DOUBLE_JUMP;
            items::double_jump::onCollect(*this);
        }
    }
}

void GameplayScene::updatePickupMessageTimer(float dt) {
    // Blendet temporare Item-Meldungen nach ihrer Anzeigedauer aus.
    if (pickupMessageTimer > 0.0f) {
        pickupMessageTimer -= dt;
        if (pickupMessageTimer < 0.0f) {
            pickupMessageTimer = 0.0f;
        }
    }
}
