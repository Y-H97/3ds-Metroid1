#include "gameplay_scene.h"

#include <algorithm>
#include <cmath>

namespace {

std::string makeVisitedKey(int x, int y) {
    return std::to_string(x) + "," + std::to_string(y);
}

float clampf(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

} // namespace

void GameplayScene::updatePlayerWorldState() {
    // Leitet Weltgitter, Tile-Position und Fog-of-War direkt aus der Player-Position ab.
    const Player& player = core.getPlayer();
    currentGridX = gridX + static_cast<int>(std::floor(player.x / 400.0f));
    currentGridY = gridY + static_cast<int>(std::floor(player.y / 240.0f));

    if (world.getCell(currentGridX, currentGridY)) {
        if (visitedCells.insert(makeVisitedKey(currentGridX, currentGridY)).second) {
            writeVisitedToDisk(activeSaveSlot);
        }
    }

    playerTileX = static_cast<int>(std::floor(player.x / 16.0f));
    playerTileY = static_cast<int>(std::floor(player.y / 16.0f));
    playerTileId = core.getMap().getTile(playerTileX, playerTileY);
}

void GameplayScene::handleMapTransition() {
    // Raumwechsel bleibt als eigenes Modul gekapselt, damit Update() schlank bleibt.
    const auto& transitions = core.getMap().getTransitions();
    bool triggered = false;
    Rect hit{};
    const Player& player = core.getPlayer();

    for (const auto& transition : transitions) {
        if (player.x < transition.x + transition.w && player.x + player.w > transition.x &&
            player.y < transition.y + transition.h && player.y + player.h > transition.y) {
            triggered = true;
            hit = transition;
            break;
        }
    }

    if (!triggered) {
        inTransition = false;
        return;
    }

    if (inTransition || !world.getCell(gridX, gridY)) {
        return;
    }

    inTransition = true;
    float txCenter = hit.x + hit.w * 0.5f;
    float tyCenter = hit.y + hit.h * 0.5f;
    std::string direction;
    if (txCenter > core.getMap().width * 16 - 32) direction = "right";
    else if (txCenter < 32) direction = "left";
    else if (tyCenter > core.getMap().height * 16 - 32) direction = "down";
    else if (tyCenter < 32) direction = "up";

    if (direction.empty()) {
        return;
    }

    float globalX = gridX * 400 + player.x;
    float globalY = gridY * 240 + player.y;
    float targetGX = globalX;
    float targetGY = globalY;
    if (direction == "right") targetGX = (gridX * 400) + core.getMap().width * 16 + 16;
    else if (direction == "left") targetGX = (gridX * 400) - 16;
    else if (direction == "down") targetGY = (gridY * 240) + core.getMap().height * 16 + 16;
    else if (direction == "up") targetGY = (gridY * 240) - 16;

    int nextGX = static_cast<int>(std::floor(targetGX / 400.0f));
    int nextGY = static_cast<int>(std::floor(targetGY / 240.0f));
    const SpatialCell* nextCell = world.getCell(nextGX, nextGY);
    if (!nextCell) {
        return;
    }

    std::string levelPath = std::string("romfs:/maps/") + nextCell->level + ".json";
    if (!core.loadMapJson(levelPath.c_str())) {
        return;
    }

    gridX = nextCell->originX;
    gridY = nextCell->originY;
    currentLevelName = nextCell->level;
    refreshMapItems();

    float offset = 24.0f;
    float localTargetX = 0.0f;
    float localTargetY = 0.0f;

    if (direction == "right") {
        localTargetX = offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - gridY) * 240.0f + screenRelY;
    } else if (direction == "left") {
        float segmentLocalX = (nextGX - gridX) * 400.0f;
        localTargetX = segmentLocalX + 400.0f - player.w - offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - gridY) * 240.0f + screenRelY;
    } else if (direction == "down") {
        localTargetY = offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - gridX) * 400.0f + screenRelX;
    } else if (direction == "up") {
        float segmentLocalY = (nextGY - gridY) * 240.0f;
        localTargetY = segmentLocalY + 240.0f - player.h - offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - gridX) * 400.0f + screenRelX;
    }

    const TileMap& map = core.getMap();
    float maxX = map.width * 16.0f - core.getPlayer().w;
    float maxY = map.height * 16.0f - core.getPlayer().h;
    localTargetX = clampf(localTargetX, 0.0f, std::max(0.0f, maxX));
    localTargetY = clampf(localTargetY, 0.0f, std::max(0.0f, maxY));

    if (world.isCheckpoint(nextGX, nextGY)) {
        checkpoint.valid = true;
        checkpoint.level = nextCell->level;
        checkpoint.gridX = nextCell->originX;
        checkpoint.gridY = nextCell->originY;
        checkpoint.x = localTargetX;
        checkpoint.y = localTargetY;
        core.setPlayerStart(localTargetX, localTargetY);
        writePersistentSaveToDisk(checkpoint, activeSaveSlot);
    } else {
        core.setPlayerPosition(localTargetX, localTargetY);
    }

    updatePlayerWorldState();
}

void GameplayScene::respawnFromCheckpoint() {
    // Tod setzt den Spieler an den letzten gueltigen Checkpoint zurueck.
    if (!core.consumeDeath()) {
        return;
    }

    if (checkpoint.valid && !checkpoint.level.empty()) {
        std::string checkpointPath = std::string("romfs:/maps/") + checkpoint.level + ".json";
        if (core.loadMapJson(checkpointPath.c_str())) {
            gridX = checkpoint.gridX;
            gridY = checkpoint.gridY;
            currentLevelName = checkpoint.level;
            refreshMapItems();
            core.setPlayerPosition(checkpoint.x, checkpoint.y);
            updatePlayerWorldState();
            return;
        }
    }

    core.setPlayerPosition(core.getPlayer().x, core.getPlayer().y);
}
