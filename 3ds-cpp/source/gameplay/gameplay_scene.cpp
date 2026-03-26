#include "gameplay_scene.h"

#ifndef DESKTOP_SIMULATOR
#include <citro2d.h>
#endif
#include <cstdio>

#include "../ui/text_renderer.h"
#include "render/bottom_ui.h"

constexpr u32 CLEAR_COLOR = C2D_Color32(16, 20, 32, 255);
constexpr u32 BG_COLOR    = C2D_Color32(10, 14, 20, 255);

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
    // Neue Karte geladen – Itemliste auffrischen und Startkollisionen neutralisieren.
    refreshMapItems();
    markItemsOverlappingPlayerAsCollected();

    checkpoint.valid = true;
    checkpoint.level = levelName;
    checkpoint.gridX = gridX;
    checkpoint.gridY = gridY;
    checkpoint.x = core.getPlayer().x;
    checkpoint.y = core.getPlayer().y;
    visitedCells.clear();
    currentGridX = gridX + static_cast<int>(core.getPlayer().x / 400.0f);
    currentGridY = gridY + static_cast<int>(core.getPlayer().y / 240.0f);
    visitedCells.insert(std::to_string(currentGridX) + "," + std::to_string(currentGridY));
    playerTileX = static_cast<int>(core.getPlayer().x / 16.0f);
    playerTileY = static_cast<int>(core.getPlayer().y / 16.0f);
    playerTileId = core.getMap().getTile(playerTileX, playerTileY);

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
    // Falls eine Item-Meldung aktiv ist, zeige sie zentral oben an
    if (pickupMessageTimer > 0.0f && !pickupMessage.empty()) {
        text.draw(200.0f, 20.0f, 0.52f, C2D_Color32(240, 220, 120, 255), "%s", pickupMessage.c_str());
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
        collectedItems,
        activeItems,
        inventorySelection,
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
