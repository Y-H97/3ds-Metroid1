#include "gameplay_scene.h"

#include <citro2d.h>
#include <cmath>
#include <cstdio>

#include "../ui/text_renderer.h"
#include "render/bottom_ui.h"

constexpr u32 CLEAR_COLOR = C2D_Color32(16, 20, 32, 255);
constexpr u32 BG_COLOR    = C2D_Color32(10, 14, 20, 255);

static std::string makeVisitedKey(int x, int y) {
    // Eindeutiger Schlüssel für besuchte Weltzellen.
    return std::to_string(x) + "," + std::to_string(y);
}

static float clampf(float v, float lo, float hi) {
    // Begrenzt einen Wert auf ein Intervall [lo, hi].
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void computeCamera(const Player& p, const TileMap& map, float tileSize, float viewW, float viewH, float& outCamX, float& outCamY) {
    // Kamera folgt dem Spieler, bleibt aber innerhalb der Kartenränder.
    float scale = tileSize / 16.0f;
    float px = p.x * scale;
    float py = p.y * scale;
    float pw = p.w * scale;
    float ph = p.h * scale;

    float mapPixelW = map.width * tileSize;
    float mapPixelH = map.height * tileSize;

    float camX = px + pw * 0.5f - viewW * 0.5f;
    float camY = py + ph * 0.5f - viewH * 0.5f;

    float maxCamX = mapPixelW - viewW;
    float maxCamY = mapPixelH - viewH;
    if (maxCamX < 0.0f) camX = maxCamX * 0.5f;
    else camX = clampf(camX, 0.0f, maxCamX);
    if (maxCamY < 0.0f) camY = maxCamY * 0.5f;
    else camY = clampf(camY, 0.0f, maxCamY);

    outCamX = camX;
    outCamY = camY;
}

static std::string pickFirstLevelFromWorld(const char* path) {
    // Fallback-Helfer: nimmt den ersten "level"-Eintrag aus world.json.
    FILE* f = fopen(path, "r");
    if (!f) return {};
    std::string s;
    char buf[512];
    while (size_t n = fread(buf, 1, sizeof(buf), f)) s.append(buf, n);
    fclose(f);

    size_t pos = s.find("\"level\"");
    if (pos == std::string::npos) return {};
    pos = s.find('"', pos + 7);
    if (pos == std::string::npos) return {};
    size_t end = s.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return s.substr(pos + 1, end - pos - 1);
}

static void renderMap(C3D_RenderTarget* target, float originX, float originY, float tileSize, const TileRenderer& renderer, const TileMap& map) {
    // Zeichnet alle nicht-leeren Kacheln der aktuellen Karte.
    C2D_SceneBegin(target);
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            uint8_t t = map.tiles[static_cast<size_t>(y * map.width + x)];
            if (t == 0) continue;
            float px = originX + x * tileSize;
            float py = originY + y * tileSize;
            renderer.drawTile(px, py, t);
        }
    }
}

bool GameplayScene::loadInitialMap() {
    // Startlogik: Welt laden, Start-Cell bestimmen, Karte laden, Spieler setzen.
    std::string levelName = pickFirstLevelFromWorld("romfs:/maps/world.json");
    bool mapOk = false;

    if (world.loadWorldJson("romfs:/maps/world.json")) {
        world.buildSpatialMap("romfs:/maps", 16, 400, 240);
        const auto& cells = world.getCells();
        if (!cells.empty()) {
            bool foundStart = false;
            for (const auto& c : cells) {
                if (c.level == "0") {
                    gridX = c.x;
                    gridY = c.y;
                    levelName = c.level;
                    foundStart = true;
                    break;
                }
            }
            if (!foundStart) {
                gridX = cells[0].x;
                gridY = cells[0].y;
                levelName = cells[0].level;
            }
        }
    }

    if (!levelName.empty()) {
        std::string p = "romfs:/maps/" + levelName + ".json";
        mapOk = core.loadMapJson(p.c_str());
    }
    if (!mapOk) {
        mapOk = core.loadMapText("romfs:/maps/demo.txt");
    }

    if (!mapOk) return false;
    currentLevelName = levelName;
    if (!core.setPlayerStartToFirstEmpty(16.0f)) {
        core.setPlayerStart(40.0f, 40.0f);
    }

    checkpoint.valid = true;
    checkpoint.level = levelName;
    checkpoint.gridX = gridX;
    checkpoint.gridY = gridY;
    checkpoint.x = core.getPlayer().x;
    checkpoint.y = core.getPlayer().y;
    visitedCells.clear();
    visitedCells.insert(makeVisitedKey(gridX, gridY));

    return true;
}

bool GameplayScene::init() {
    // Szene auf definierten Anfangszustand setzen.
    requestExit = false;
    requestMenu = false;
    inTransition = false;
    showFpsEnabled = false;
    fpsTimerMs = osGetTime();
    fpsFrameCounter = 0;
    fpsValue = 0;
    debugScrollPx = 0;
    debugMaxScrollPx = 0;

    renderer.init("romfs:/gfx/tiles.t3x", 16);
    if (!loadInitialMap()) return false;
    activeSaveSlot = 1;
    loadPersistentSaveFromDisk(activeSaveSlot);
    loadVisitedFromDisk(activeSaveSlot);
    return true;
}

void GameplayScene::shutdown() {
    // Beim Beenden letzten sinnvollen Stand persistieren.
    if (!currentLevelName.empty()) {
        Checkpoint saveToPersist{};
        if (checkpoint.valid && !checkpoint.level.empty()) {
            saveToPersist = checkpoint;
        } else {
            saveToPersist.valid = true;
            saveToPersist.level = currentLevelName;
            saveToPersist.gridX = gridX;
            saveToPersist.gridY = gridY;
            saveToPersist.x = core.getPlayer().x;
            saveToPersist.y = core.getPlayer().y;
        }
        writePersistentSaveToDisk(saveToPersist, activeSaveSlot);
        writeVisitedToDisk(activeSaveSlot);
    }
    renderer.shutdown();
}

void GameplayScene::update(float dt) {
    // Haupt-Update: Eingabe anwenden, Raumwechsel prüfen, Tod/Respawn, FPS zählen.
    InputState input;
    input.left = moveLeftHeld;
    input.right = moveRightHeld;
    input.jump = jumpHeld;
    input.jumpPressed = jumpPressed;
    core.update(input, dt);
    jumpPressed = false;

    const Player& currentPlayer = core.getPlayer();
    currentGridX = gridX + static_cast<int>(std::floor(currentPlayer.x / 400.0f));
    currentGridY = gridY + static_cast<int>(std::floor(currentPlayer.y / 240.0f));
    if (world.getCell(currentGridX, currentGridY)) {
        if (visitedCells.insert(makeVisitedKey(currentGridX, currentGridY)).second) {
            writeVisitedToDisk(activeSaveSlot);
        }
    }
    playerTileX = static_cast<int>(std::floor(currentPlayer.x / 16.0f));
    playerTileY = static_cast<int>(std::floor(currentPlayer.y / 16.0f));
    playerTileId = core.getMap().getTile(playerTileX, playerTileY);

    // Prüfen, ob der Spieler einen Übergangstrigger berührt.
    const auto& transitions = core.getMap().getTransitions();
    bool triggered = false;
    Rect hit{};
    const Player& player = core.getPlayer();
    for (const auto& tr : transitions) {
        if (player.x < tr.x + tr.w && player.x + player.w > tr.x && player.y < tr.y + tr.h && player.y + player.h > tr.y) {
            triggered = true;
            hit = tr;
            break;
        }
    }
    if (!triggered) inTransition = false;
    // Bei neuem Übergang Zielraum bestimmen und ggf. laden.
    if (triggered && !inTransition && world.getCell(gridX, gridY)) {
        inTransition = true;
        float txCenter = hit.x + hit.w * 0.5f;
        float tyCenter = hit.y + hit.h * 0.5f;
        std::string dir;
        if (txCenter > core.getMap().width * 16 - 32) dir = "right";
        else if (txCenter < 32) dir = "left";
        else if (tyCenter > core.getMap().height * 16 - 32) dir = "down";
        else if (tyCenter < 32) dir = "up";

        if (!dir.empty()) {
            float globalX = gridX * 400 + player.x;
            float globalY = gridY * 240 + player.y;
            float targetGX = globalX;
            float targetGY = globalY;
            if (dir == "right") targetGX = (gridX * 400) + core.getMap().width * 16 + 16;
            else if (dir == "left") targetGX = (gridX * 400) - 16;
            else if (dir == "down") targetGY = (gridY * 240) + core.getMap().height * 16 + 16;
            else if (dir == "up") targetGY = (gridY * 240) - 16;

            int nextGX = static_cast<int>(std::floor(targetGX / 400.0f));
            int nextGY = static_cast<int>(std::floor(targetGY / 240.0f));
            const SpatialCell* nextCell = world.getCell(nextGX, nextGY);
            if (nextCell) {
                std::string levelPath = std::string("romfs:/maps/") + nextCell->level + ".json";
                if (core.loadMapJson(levelPath.c_str())) {
                    gridX = nextCell->originX;
                    gridY = nextCell->originY;
                    currentLevelName = nextCell->level;

                    float offset = 24.0f;
                    float localTargetX = 0.0f;
                    float localTargetY = 0.0f;

                    if (dir == "right") {
                        localTargetX = offset;
                        float screenRelY = std::fmod(player.y, 240.0f);
                        localTargetY = (std::floor(targetGY / 240.0f) - gridY) * 240.0f + screenRelY;
                    } else if (dir == "left") {
                        float segmentLocalX = (nextGX - gridX) * 400.0f;
                        localTargetX = segmentLocalX + 400.0f - player.w - offset;
                        float screenRelY = std::fmod(player.y, 240.0f);
                        localTargetY = (std::floor(targetGY / 240.0f) - gridY) * 240.0f + screenRelY;
                    } else if (dir == "down") {
                        localTargetY = offset;
                        float screenRelX = std::fmod(player.x, 400.0f);
                        localTargetX = (std::floor(targetGX / 400.0f) - gridX) * 400.0f + screenRelX;
                    } else if (dir == "up") {
                        float segmentLocalY = (nextGY - gridY) * 240.0f;
                        localTargetY = segmentLocalY + 240.0f - player.h - offset;
                        float screenRelX = std::fmod(player.x, 400.0f);
                        localTargetX = (std::floor(targetGX / 400.0f) - gridX) * 400.0f + screenRelX;
                    }

                    // Checkpoint-Räume aktualisieren Spawnpunkt und speichern sofort.
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
                }
            }
        }
    }

    // Tod führt zum Respawn am letzten gültigen Checkpoint.
    if (core.consumeDeath()) {
        if (checkpoint.valid && !checkpoint.level.empty()) {
            std::string cpPath = std::string("romfs:/maps/") + checkpoint.level + ".json";
            if (core.loadMapJson(cpPath.c_str())) {
                gridX = checkpoint.gridX;
                gridY = checkpoint.gridY;
                core.setPlayerPosition(checkpoint.x, checkpoint.y);
            }
        } else {
            core.setPlayerPosition(core.getPlayer().x, core.getPlayer().y);
        }
    }

    fpsFrameCounter++;
    u64 nowMs = osGetTime();
    if (nowMs - fpsTimerMs >= 1000) {
        fpsValue = fpsFrameCounter;
        fpsFrameCounter = 0;
        fpsTimerMs = nowMs;
    }
}

void GameplayScene::renderTop(C3D_RenderTarget* top, TextRenderer& text, bool debugInfoEnabled) {
    // Top-Screen: Karte, Spieler, Trigger und optionale Debug-/FPS-Texte.
    float camX = 0.0f;
    float camY = 0.0f;
    computeCamera(core.getPlayer(), core.getMap(), 16.0f, 400.0f, 240.0f, camX, camY);
    lastCamX = camX;
    lastCamY = camY;

    C2D_TargetClear(top, CLEAR_COLOR);
    renderMap(top, -camX, -camY, 16.0f, renderer, core.getMap());

    const Player& p = core.getPlayer();
    C2D_DrawRectSolid(p.x - camX, p.y - camY, 0.1f, p.w, p.h, C2D_Color32(250, 210, 60, 255));
    for (const auto& tr : core.getMap().getTransitions()) {
        C2D_DrawRectSolid(tr.x - camX, tr.y - camY, 0.2f, tr.w, tr.h, C2D_Color32(0, 220, 0, 255));
    }
    if (showFpsEnabled) {
        text.draw(6.0f, 6.0f, 0.36f, C2D_Color32(120, 255, 140, 255), "FPS: %d", fpsValue);
    }
    if (debugInfoEnabled) {
        const Player& p = core.getPlayer();
        text.draw(6.0f, 22.0f, 0.30f, C2D_Color32(180, 210, 255, 255), "Lvl:%s Grid:%d,%d O:%d,%d", currentLevelName.c_str(), currentGridX, currentGridY, gridX, gridY);
        text.draw(6.0f, 36.0f, 0.30f, C2D_Color32(180, 210, 255, 255), "Pos:%.1f/%.1f Vel:%.1f/%.1f", p.x, p.y, p.vx, p.vy);
        text.draw(6.0f, 50.0f, 0.30f, C2D_Color32(180, 210, 255, 255), "Tile:%d,%d id:%d G:%s T:%s", playerTileX, playerTileY, playerTileId, p.grounded ? "Y" : "N", inTransition ? "Y" : "N");
    }
}

void GameplayScene::renderBottom(C3D_RenderTarget* bottom, TextRenderer& text, bool debugInfoEnabled) {
    // Bottom-Screen: delegiert an Tab-Renderer (Map/Inventar/Settings/Debug).
    C2D_TargetClear(bottom, BG_COLOR);
    C2D_SceneBegin(bottom);
    renderGameplayBottomUI(
        text,
        world,
        visitedCells,
        core,
        bottomMode,
        settingsSelection,
        showFpsEnabled,
        debugInfoEnabled,
        currentGridX,
        currentGridY,
        gridX,
        gridY,
        fpsValue,
        lastCamX,
        lastCamY,
        currentLevelName.c_str(),
        inTransition,
        checkpoint.valid,
        checkpoint.gridX,
        checkpoint.gridY,
        playerTileX,
        playerTileY,
        playerTileId,
        debugScrollPx,
        debugMaxScrollPx
    );

    if (debugScrollPx < 0) debugScrollPx = 0;
    if (debugScrollPx > debugMaxScrollPx) debugScrollPx = debugMaxScrollPx;
}

bool GameplayScene::shouldExitGame() const {
    return requestExit;
}

bool GameplayScene::shouldReturnToMenu() const {
    return requestMenu;
}

void GameplayScene::clearReturnToMenu() {
    requestMenu = false;
}
