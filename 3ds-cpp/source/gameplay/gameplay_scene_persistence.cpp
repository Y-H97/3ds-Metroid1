#include "gameplay_scene.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace {

constexpr const char* SAVE_DIR = "sdmc:/3ds/3ds-cpp";
constexpr const char* SAVE_PATH_LEGACY = "sdmc:/3ds/3ds-cpp/savegame.dat";
constexpr const char* VISITED_PATH_LEGACY = "sdmc:/3ds/3ds-cpp/visited.dat";

int normalizeSlot(int slot) {
    // Erlaubt nur gültige Save-Slots (1..SAVE_SLOT_COUNT).
    if (slot < 1) return 1;
    if (slot > GameplayScene::SAVE_SLOT_COUNT) return GameplayScene::SAVE_SLOT_COUNT;
    return slot;
}

std::string makeSavePath(int slot) {
    // Dateiname für Slot-spezifischen Spielstand.
    char path[128];
    std::snprintf(path, sizeof(path), "sdmc:/3ds/3ds-cpp/savegame_slot%d.dat", normalizeSlot(slot));
    return path;
}

std::string makeVisitedPath(int slot) {
    // Dateiname für Slot-spezifische besuchte Kartenzellen.
    char path[128];
    std::snprintf(path, sizeof(path), "sdmc:/3ds/3ds-cpp/visited_slot%d.dat", normalizeSlot(slot));
    return path;
}

std::string makeVisitedKey(int x, int y) {
    return std::to_string(x) + "," + std::to_string(y);
}

} // namespace

void GameplayScene::setActiveSaveSlot(int slot) {
    // Aktiven Slot robust setzen (inkl. Begrenzung).
    activeSaveSlot = normalizeSlot(slot);
}

int GameplayScene::getActiveSaveSlot() const {
    return activeSaveSlot;
}

bool GameplayScene::hasPersistentSave(int slot) {
    PersistentSave temp{};
    return loadPersistentSaveFromDisk(slot, &temp);
}

bool GameplayScene::loadPersistentSaveFromDisk(int slot, PersistentSave* outSave) {
    // Lädt Save-Daten; unterstützt neues und altes Dateiformat.
    PersistentSave loaded{};

    std::string savePath = makeSavePath(slot);
    FILE* f = fopen(savePath.c_str(), "rb");
    if (!f && normalizeSlot(slot) == 1) {
        f = fopen(SAVE_PATH_LEGACY, "rb");
    }

    if (!f) return false;

    char levelBuf[128] = {0};
    char lineBuf[256] = {0};
    int gx = 0;
    int gy = 0;
    float x = 0.0f;
    float y = 0.0f;

    bool parsed = false;

    if (std::fgets(levelBuf, sizeof(levelBuf), f) && std::fgets(lineBuf, sizeof(lineBuf), f)) {
        size_t len = std::strlen(levelBuf);
        while (len > 0 && (levelBuf[len - 1] == '\n' || levelBuf[len - 1] == '\r')) {
            levelBuf[len - 1] = '\0';
            --len;
        }
        int got = std::sscanf(lineBuf, " %d %d %f %f", &gx, &gy, &x, &y);
        if (len > 0 && got == 4) parsed = true;
    }

    if (!parsed) {
        std::rewind(f);
        int gotLegacy = std::fscanf(f, " %127s %d %d %f %f", levelBuf, &gx, &gy, &x, &y);
        if (gotLegacy == 5) parsed = true;
    }

    fclose(f);
    if (!parsed) return false;

    loaded.valid = true;
    loaded.level = levelBuf;
    loaded.gridX = gx;
    loaded.gridY = gy;
    loaded.x = x;
    loaded.y = y;

    if (outSave) {
        *outSave = loaded;
    } else {
        persistentSave = loaded;
    }
    return true;
}

bool GameplayScene::writePersistentSaveToDisk(const Checkpoint& cp, int slot) {
    // Schreibt Checkpoint als persistenten Slot-Spielstand.
    if (!cp.valid || cp.level.empty()) return false;

    mkdir("sdmc:/3ds", 0777);
    mkdir(SAVE_DIR, 0777);

    std::string savePath = makeSavePath(slot);
    FILE* f = fopen(savePath.c_str(), "wb");
    if (!f) return false;

    std::fprintf(f, "%s\n%d %d %.3f %.3f\n", cp.level.c_str(), cp.gridX, cp.gridY, cp.x, cp.y);
    fclose(f);

    persistentSave.valid = true;
    persistentSave.level = cp.level;
    persistentSave.gridX = cp.gridX;
    persistentSave.gridY = cp.gridY;
    persistentSave.x = cp.x;
    persistentSave.y = cp.y;
    return true;
}

bool GameplayScene::loadVisitedFromDisk(int slot) {
    // Lädt Fog-of-War-Zustand (besuchte Weltzellen).
    std::string visitedPath = makeVisitedPath(slot);
    FILE* f = fopen(visitedPath.c_str(), "rb");
    if (!f && normalizeSlot(slot) == 1) {
        f = fopen(VISITED_PATH_LEGACY, "rb");
    }
    if (!f) return false;

    std::unordered_set<std::string> loaded;
    int x = 0;
    int y = 0;
    while (std::fscanf(f, " %d,%d", &x, &y) == 2) {
        loaded.insert(makeVisitedKey(x, y));
    }
    fclose(f);

    if (!loaded.empty()) {
        visitedCells = std::move(loaded);
        return true;
    }
    return false;
}

bool GameplayScene::writeVisitedToDisk(int slot) const {
    // Speichert alle besuchten Zellen zeilenweise als "x,y".
    mkdir("sdmc:/3ds", 0777);
    mkdir(SAVE_DIR, 0777);

    std::string visitedPath = makeVisitedPath(slot);
    FILE* f = fopen(visitedPath.c_str(), "wb");
    if (!f) return false;

    for (const auto& key : visitedCells) {
        std::fprintf(f, "%s\n", key.c_str());
    }
    fclose(f);
    return true;
}

bool GameplayScene::resetVisitedProgress(int slot) {
    // Löscht den Kartenfortschritt des Slots (Datei + RAM-Zustand).
    visitedCells.clear();
    std::string visitedPath = makeVisitedPath(slot);
    int rc = std::remove(visitedPath.c_str());
    if (rc != 0 && normalizeSlot(slot) == 1) {
        std::remove(VISITED_PATH_LEGACY);
    }
    return rc == 0 || rc == -1;
}

bool GameplayScene::startNewGame(int slot) {
    // Startet wirklich neu: Save löschen, Visited löschen, Initialmap laden.
    activeSaveSlot = normalizeSlot(slot);
    requestMenu = false;
    requestExit = false;
    inTransition = false;
    checkpoint = {};
    persistentSave = {};
    std::string savePath = makeSavePath(activeSaveSlot);
    std::remove(savePath.c_str());
    if (activeSaveSlot == 1) {
        std::remove(SAVE_PATH_LEGACY);
    }
    resetVisitedProgress(activeSaveSlot);

    if (!loadInitialMap()) return false;
    writeVisitedToDisk(activeSaveSlot);
    return true;
}

bool GameplayScene::loadFromCheckpoint(int slot) {
    // Lädt den letzten persistierten Stand eines Slots.
    activeSaveSlot = normalizeSlot(slot);
    persistentSave = {};
    if (!loadPersistentSaveFromDisk(activeSaveSlot)) return false;
    if (!persistentSave.valid || persistentSave.level.empty()) return false;

    std::string cpPath = std::string("romfs:/maps/") + persistentSave.level + ".json";
    if (!core.loadMapJson(cpPath.c_str())) return false;

    gridX = persistentSave.gridX;
    gridY = persistentSave.gridY;
    currentLevelName = persistentSave.level;
    core.setPlayerPosition(persistentSave.x, persistentSave.y);

    checkpoint.valid = true;
    checkpoint.level = persistentSave.level;
    checkpoint.gridX = persistentSave.gridX;
    checkpoint.gridY = persistentSave.gridY;
    checkpoint.x = persistentSave.x;
    checkpoint.y = persistentSave.y;

    const Player& p = core.getPlayer();
    currentGridX = gridX + static_cast<int>(std::floor(p.x / 400.0f));
    currentGridY = gridY + static_cast<int>(std::floor(p.y / 240.0f));
    playerTileX = static_cast<int>(std::floor(p.x / 16.0f));
    playerTileY = static_cast<int>(std::floor(p.y / 16.0f));
    playerTileId = core.getMap().getTile(playerTileX, playerTileY);
    inTransition = false;
    if (visitedCells.insert(makeVisitedKey(currentGridX, currentGridY)).second) {
        writeVisitedToDisk(activeSaveSlot);
    }

    return true;
}
