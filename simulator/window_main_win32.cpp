#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <direct.h>
#include <mmsystem.h>
#include <sys/stat.h>
#include <string>
#include <unordered_set>
#include <vector>

#define DESKTOP_SIMULATOR 1
#include "compat/desktop_3ds_compat.h"
#include "../3ds-cpp/source/core/game_core.h"
#include "../3ds-cpp/source/core/world_map.h"
#include "../3ds-cpp/source/gameplay/gameplay_scene.h" // für ITEM_DOUBLE_JUMP und strukturelle Deklarationen
#include "../3ds-cpp/source/gameplay/items/double_jump/double_jump.h" // Zugriff auf item-ID
#include "../3ds-cpp/source/menu/controllers/main_menu_controller.h"
#include "../3ds-cpp/source/menu/views/manual_content.h"

namespace {

// Hauptzustände der Anwendung: Menü oder aktives Gameplay.
enum AppState {
    APP_MAIN_MENU = 0,
    APP_GAME = 1,
};

enum BottomTab {
    TAB_MAP = 0,
    TAB_INVENTORY = 1,
    TAB_SETTINGS = 2,
    TAB_DEBUG = 3,
};

struct Checkpoint {
    bool valid = false;
    std::string level;
    int gridX = 0;
    int gridY = 0;
    float x = 0.0f;
    float y = 0.0f;
};

struct PersistentSave {
    bool valid = false;
    std::string level;
    int gridX = 0;
    int gridY = 0;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t collectedItems = 0;
    uint32_t activeItems = 0;
};

struct RectI {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct App {
    // Zentraler Laufzeitzustand des Desktop-Simulators.
    HWND hwnd = nullptr;
    AppState appState = APP_MAIN_MENU;

    MainMenuController menu;
    GameCore core;
    WorldMap world;

    std::string mapsRoot;
    std::string saveRoot;

    int activeSaveSlot = 1;
    int gridX = 0;
    int gridY = 0;
    int currentGridX = 0;
    int currentGridY = 0;
    int lastCheckpointGridX = 0;
    int lastCheckpointGridY = 0;
    bool lastCheckpointGridValid = false;
    std::string currentLevelName;

    // item state
    uint32_t collectedItems = 0;
    uint32_t activeItems = 0;
    int inventorySelection = 0;
    std::string pickupMessage;
    float pickupMessageTimer = 0.0f;

    struct MapItem {
        std::string type;
        float x = 0.0f;
        float y = 0.0f;
        bool collected = false;
    };
    std::vector<MapItem> mapItems;

    Checkpoint checkpoint;
    PersistentSave persistentSave;
    std::unordered_set<std::string> visitedCells;

    int bottomMode = TAB_MAP;
    int settingsSelection = 0;
    bool showFpsEnabled = false;
    bool requestMenu = false;
    bool inTransition = false;

    bool savedDebugEnabled = true;
    bool savedControlsSwapped = false;
    bool savedShowFps = false;

    bool controlsSwapped = false;
    bool moveLeftHeld = false;
    bool moveRightHeld = false;
    bool jumpHeld = false;
    bool jumpPressed = false;

    int fpsValue = 0;
    int fpsFrameCounter = 0;
    u64 fpsTimerMs = 0;
    int debugScrollPx = 0;
    int debugMaxScrollPx = 0;

    u32 keyHeld = 0;
    u32 keyDown = 0;

    bool touchPending = false;
    touchPosition pendingTouch{};

    float camX = 0.0f;
    float camY = 0.0f;

    RectI renderArea{0, 0, 420, 520};
    RectI topView{10, 10, 400, 240};
    RectI bottomView{10, 270, 320, 240};

    HDC backDc = nullptr;
    HBITMAP backBmp = nullptr;
    HBITMAP backOldBmp = nullptr;
    int backW = 0;
    int backH = 0;

    HDC mapDc = nullptr;
    HBITMAP mapBmp = nullptr;
    HBITMAP mapOldBmp = nullptr;
    int mapW = 0;
    int mapH = 0;
} g;

bool loadPersistentSaveFromDisk(int slot, PersistentSave& outSave);
void releaseMapCache();
bool rebuildMapCache();
void fillRect(HDC hdc, int x, int y, int w, int h, COLORREF color);
bool ensureBackBuffer(HDC referenceDc, int width, int height);

u64 nowMs() {
    // Aktuelle Zeit in Millisekunden (für FPS-Messung etc.).
    return static_cast<u64>(GetTickCount64());
}

std::string makeCellKey(int x, int y) {
    // Einheitlicher Schlüssel für Weltzellen: "x,y".
    return std::to_string(x) + "," + std::to_string(y);
}

std::string normalizeSlashes(std::string path) {
    for (char& ch : path) {
        if (ch == '\\') ch = '/';
    }
    return path;
}

std::string parentDir(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return {};
    return path.substr(0, pos);
}

void ensureDirRecursive(const std::string& inPath) {
    // Erstellt alle Verzeichnisse im Pfad (mkdir -p Verhalten).
    std::string path = normalizeSlashes(inPath);
    if (path.empty()) return;

    size_t start = 0;
    if (path.size() >= 2 && path[1] == ':') start = 3;
    if (path.rfind("//", 0) == 0) {
        size_t p = path.find('/', 2);
        if (p == std::string::npos) return;
        p = path.find('/', p + 1);
        if (p == std::string::npos) return;
        start = p + 1;
    }

    for (size_t i = start; i < path.size(); ++i) {
        if (path[i] != '/') continue;
        std::string part = path.substr(0, i);
        if (!part.empty()) _mkdir(part.c_str());
    }
    _mkdir(path.c_str());
}

std::string executableDir() {
    char buffer[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return ".";
    return parentDir(buffer);
}

std::string localAppDataDir() {
    const char* localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData && localAppData[0] != '\0') {
        return normalizeSlashes(localAppData);
    }
    return normalizeSlashes(executableDir());
}

void ensureSaveDir() {
    ensureDirRecursive(g.saveRoot);
}

std::string savePathForSlot(int slot) {
    return g.saveRoot + "/savegame_slot" + std::to_string(slot) + ".dat";
}

std::string visitedPathForSlot(int slot) {
    return g.saveRoot + "/visited_slot" + std::to_string(slot) + ".dat";
}

std::string settingsPath() {
    return g.saveRoot + "/settings.dat";
}

bool loadSettingsFromDisk(bool& outDebugEnabled, bool& outControlsSwapped, bool& outShowFps) {
    // Lädt persistente Einstellungen aus Datei.
    FILE* f = std::fopen(settingsPath().c_str(), "rb");
    if (!f) return false;

    int debug = 1;
    int controls = 0;
    int fps = 0;
    int got = std::fscanf(f, " %d %d %d", &debug, &controls, &fps);
    std::fclose(f);
    if (got != 3) return false;

    outDebugEnabled = debug != 0;
    outControlsSwapped = controls != 0;
    outShowFps = fps != 0;
    return true;
}

bool saveSettingsToDisk(bool debugEnabled, bool controlsSwapped, bool showFps) {
    // Speichert persistente Einstellungen als drei 0/1-Werte.
    ensureSaveDir();
    FILE* f = std::fopen(settingsPath().c_str(), "wb");
    if (!f) return false;

    std::fprintf(f, "%d %d %d\n", debugEnabled ? 1 : 0, controlsSwapped ? 1 : 0, showFps ? 1 : 0);
    std::fclose(f);
    return true;
}

void syncSettingsToDiskIfChanged() {
    // Nur schreiben, wenn sich Werte seit letztem bekannten Zustand geändert haben.
    bool debugEnabled = g.menu.getDebugEnabled();
    bool controlsSwapped = g.menu.getControlsSwapped();
    bool showFps = g.showFpsEnabled;

    if (debugEnabled == g.savedDebugEnabled &&
        controlsSwapped == g.savedControlsSwapped &&
        showFps == g.savedShowFps) {
        return;
    }

    if (saveSettingsToDisk(debugEnabled, controlsSwapped, showFps)) {
        g.savedDebugEnabled = debugEnabled;
        g.savedControlsSwapped = controlsSwapped;
        g.savedShowFps = showFps;
    }
}

bool hasPersistentSave(int slot) {
    PersistentSave tmp{};
    return loadPersistentSaveFromDisk(slot, tmp);
}

bool loadPersistentSaveFromDisk(int slot, PersistentSave& outSave) {
    // Lädt Savegame eines Slots (unterstützt neues und Legacy-Format).
    FILE* f = std::fopen(savePathForSlot(slot).c_str(), "rb");
    if (!f) return false;

    char levelBuf[128] = {0};
    int gx = 0;
    int gy = 0;
    float x = 0.0f;
    float y = 0.0f;

    bool ok = false;
    if (std::fgets(levelBuf, sizeof(levelBuf), f)) {
        char lineBuf[256] = {0};
        if (std::fgets(lineBuf, sizeof(lineBuf), f)) {
            size_t len = std::strlen(levelBuf);
            while (len > 0 && (levelBuf[len - 1] == '\n' || levelBuf[len - 1] == '\r')) {
                levelBuf[len - 1] = '\0';
                --len;
            }
            if (len > 0 && std::sscanf(lineBuf, " %d %d %f %f", &gx, &gy, &x, &y) == 4) {
                ok = true;
            }
        }
    }

    if (!ok) {
        std::rewind(f);
        if (std::fscanf(f, " %127s %d %d %f %f", levelBuf, &gx, &gy, &x, &y) == 5) {
            ok = true;
        }
    }

    // wenn noch eine dritte Zeile mit Items existiert, einlesen
    if (ok) {
        char itemsBuf[256] = {0};
        if (std::fgets(itemsBuf, sizeof(itemsBuf), f)) {
            uint32_t col=0, act=0;
            if (std::sscanf(itemsBuf, " items %u %u", &col, &act) == 2) {
                outSave.collectedItems = col;
                outSave.activeItems = act;
            }
        }
    }

    std::fclose(f);
    if (!ok) return false;

    outSave.valid = true;
    outSave.level = levelBuf;
    outSave.gridX = gx;
    outSave.gridY = gy;
    outSave.x = x;
    outSave.y = y;
    return true;
}

bool writePersistentSaveToDisk(const Checkpoint& cp, int slot) {
    // Schreibt den aktuellen Checkpoint als Slot-Spielstand.
    if (!cp.valid || cp.level.empty()) return false;
    ensureSaveDir();

    FILE* f = std::fopen(savePathForSlot(slot).c_str(), "wb");
    if (!f) return false;
    std::fprintf(f, "%s\n%d %d %.3f %.3f\n", cp.level.c_str(), cp.gridX, cp.gridY, cp.x, cp.y);
    // Item-Flags aus globalem Zustand hinzufügen
    std::fprintf(f, "items %u %u\n", g.collectedItems, g.activeItems);
    std::fclose(f);
    return true;
}

bool loadVisitedFromDisk(int slot) {
    // Lädt besuchte Weltzellen (Fog-of-War) für den Slot.
    FILE* f = std::fopen(visitedPathForSlot(slot).c_str(), "rb");
    if (!f) return false;
    std::unordered_set<std::string> loaded;
    int x = 0;
    int y = 0;
    while (std::fscanf(f, " %d,%d", &x, &y) == 2) {
        loaded.insert(makeCellKey(x, y));
    }
    std::fclose(f);
    if (loaded.empty()) return false;
    g.visitedCells = std::move(loaded);
    return true;
}

bool writeVisitedToDisk(int slot) {
    // Persistiert besuchte Weltzellen für den Slot.
    ensureSaveDir();
    FILE* f = std::fopen(visitedPathForSlot(slot).c_str(), "wb");
    if (!f) return false;
    for (const auto& key : g.visitedCells) {
        std::fprintf(f, "%s\n", key.c_str());
    }
    std::fclose(f);
    return true;
}

void resetVisitedProgress(int slot) {
    g.visitedCells.clear();
    std::remove(visitedPathForSlot(slot).c_str());
}

void persistRuntimeState() {
    // Sichert beim Verlassen den letzten sinnvollen Stand.
    if (g.currentLevelName.empty()) return;

    Checkpoint saveToPersist{};
    if (g.checkpoint.valid && !g.checkpoint.level.empty()) {
        saveToPersist = g.checkpoint;
    } else {
        saveToPersist.valid = true;
        saveToPersist.level = g.currentLevelName;
        saveToPersist.gridX = g.gridX;
        saveToPersist.gridY = g.gridY;
        saveToPersist.x = g.core.getPlayer().x;
        saveToPersist.y = g.core.getPlayer().y;
    }

    writePersistentSaveToDisk(saveToPersist, g.activeSaveSlot);
    writeVisitedToDisk(g.activeSaveSlot);
}

void updateCheckpointFromCurrentCell() {
    // Aktualisiert Checkpoint, wenn aktuelle Weltzelle als Checkpoint markiert ist.
    if (!g.world.isCheckpoint(g.currentGridX, g.currentGridY)) {
        return;
    }

    const Player& p = g.core.getPlayer();
    bool changedCell = !g.lastCheckpointGridValid ||
        g.lastCheckpointGridX != g.currentGridX ||
        g.lastCheckpointGridY != g.currentGridY;

    if (!changedCell) {
        return;
    }

    g.checkpoint.valid = true;
    g.checkpoint.level = g.currentLevelName;
    g.checkpoint.gridX = g.gridX;
    g.checkpoint.gridY = g.gridY;
    g.checkpoint.x = p.x;
    g.checkpoint.y = p.y;

    g.lastCheckpointGridValid = true;
    g.lastCheckpointGridX = g.currentGridX;
    g.lastCheckpointGridY = g.currentGridY;

    writePersistentSaveToDisk(g.checkpoint, g.activeSaveSlot);
}

float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

COLORREF colorForTile(int tile) {
    switch (tile) {
        case 1:
            return RGB(128, 128, 204);
        case 2:
            return RGB(230, 51, 51);
        case 4:
            return RGB(51, 51, 51);
        case 5:
            return RGB(26, 26, 77);
        case 6:
            return RGB(26, 51, 26);
        case 7:
            return RGB(51, 26, 51);
        case 8:
            return RGB(38, 38, 38);
        case 9:
            return RGB(20, 20, 20);
        case 10:
            return RGB(46, 56, 64);
        case 11:
            return RGB(13, 13, 51);
        case 12:
            return RGB(0, 51, 64);
        case 13:
            return RGB(13, 38, 13);
        case 14:
            return RGB(51, 51, 26);
        case 15:
            return RGB(77, 26, 26);
        case 16:
            return RGB(77, 51, 26);
        case 17:
            return RGB(51, 38, 20);
        case 18:
            return RGB(64, 26, 13);
        case 19:
            return RGB(0, 64, 64);
        case 20:
            return RGB(38, 46, 31);
        case 21:
            return RGB(89, 38, 26);
        case 3:
            return RGB(90, 180, 220);
        case 30:
        case 31:
            return RGB(153, 153, 230);
        case 32:
        case 33:
            return RGB(102, 102, 179);
        default:
            return RGB(90, 180, 220);
    }
}

void releaseMapCache() {
    if (g.mapDc && g.mapBmp) {
        if (g.mapOldBmp) {
            SelectObject(g.mapDc, g.mapOldBmp);
            g.mapOldBmp = nullptr;
        }
        DeleteObject(g.mapBmp);
        g.mapBmp = nullptr;
    }
    if (g.mapDc) {
        DeleteDC(g.mapDc);
        g.mapDc = nullptr;
    }
    g.mapW = 0;
    g.mapH = 0;
}

void fillTriangle(HDC hdc, POINT p0, POINT p1, POINT p2, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    POINT points[3] = {p0, p1, p2};
    Polygon(hdc, points, 3);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void drawSlopeTileToCache(HDC hdc, int x, int y, int tileIndex) {
    const int s = 16;
    COLORREF colMain = RGB(153, 153, 230);
    COLORREF colShade = RGB(118, 118, 188);
    COLORREF lineCol = RGB(198, 198, 250);

    if (tileIndex == 32 || tileIndex == 33) {
        colMain = RGB(122, 122, 194);
        colShade = RGB(94, 94, 156);
        lineCol = RGB(170, 170, 228);
    }

    fillRect(hdc, x, y, s, s, RGB(28, 34, 50));

    if (tileIndex == 30) {
        fillTriangle(hdc, {x, y + s}, {x + s, y + s}, {x + s, y}, colMain);
        fillTriangle(hdc, {x + s / 2, y + s}, {x + s, y + s}, {x + s, y + s / 2}, colShade);
        HPEN pen = CreatePen(PS_SOLID, 1, lineCol);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        MoveToEx(hdc, x + s, y, nullptr);
        LineTo(hdc, x, y + s);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    } else if (tileIndex == 31) {
        fillTriangle(hdc, {x, y}, {x, y + s}, {x + s, y + s}, colMain);
        fillTriangle(hdc, {x, y + s / 2}, {x, y + s}, {x + s / 2, y + s}, colShade);
        HPEN pen = CreatePen(PS_SOLID, 1, lineCol);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        MoveToEx(hdc, x, y, nullptr);
        LineTo(hdc, x + s, y + s);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    } else if (tileIndex == 32) {
        fillTriangle(hdc, {x, y}, {x + s, y}, {x + s, y + s}, colMain);
        fillTriangle(hdc, {x + s / 2, y}, {x + s, y}, {x + s, y + s / 2}, colShade);
        HPEN pen = CreatePen(PS_SOLID, 1, lineCol);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        MoveToEx(hdc, x, y, nullptr);
        LineTo(hdc, x + s, y + s);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    } else if (tileIndex == 33) {
        fillTriangle(hdc, {x, y}, {x + s, y}, {x, y + s}, colMain);
        fillTriangle(hdc, {x, y}, {x + s / 2, y}, {x, y + s / 2}, colShade);
        HPEN pen = CreatePen(PS_SOLID, 1, lineCol);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        MoveToEx(hdc, x + s, y, nullptr);
        LineTo(hdc, x, y + s);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}

bool rebuildMapCache() {
    // Rendert die aktuelle Tilemap in ein Offscreen-Bitmap für schnelle Ausgabe.
    const TileMap& map = g.core.getMap();
    if (map.width <= 0 || map.height <= 0) {
        releaseMapCache();
        return false;
    }

    releaseMapCache();

    HDC screenDc = GetDC(g.hwnd);
    if (!screenDc) return false;

    g.mapDc = CreateCompatibleDC(screenDc);
    if (!g.mapDc) {
        ReleaseDC(g.hwnd, screenDc);
        return false;
    }

    g.mapW = map.width * 16;
    g.mapH = map.height * 16;
    g.mapBmp = CreateCompatibleBitmap(screenDc, g.mapW, g.mapH);
    ReleaseDC(g.hwnd, screenDc);
    if (!g.mapBmp) {
        releaseMapCache();
        return false;
    }

    g.mapOldBmp = static_cast<HBITMAP>(SelectObject(g.mapDc, g.mapBmp));

    fillRect(g.mapDc, 0, 0, g.mapW, g.mapH, RGB(18, 22, 34));
    for (int y = 0; y < map.height; ++y) {
        for (int x = 0; x < map.width; ++x) {
            int tile = map.getTile(x, y);
            if (tile <= 0) continue;
            if (tile == 30 || tile == 31 || tile == 32 || tile == 33) {
                drawSlopeTileToCache(g.mapDc, x * 16, y * 16, tile);
            } else {
                fillRect(g.mapDc, x * 16, y * 16, 16, 16, colorForTile(tile));
            }
        }
    }
    // Items ebenfalls in Cache zeichnen (blaue Quadrate)
    for (const auto& it : map.items) {
        // überspringen, wenn bereits persistent gesammelt
        if (it.type == items::double_jump::id() && (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP)) continue;
        fillRect(g.mapDc, it.x * 16, it.y * 16, 16, 16, RGB(0,0,255));
    }

    return true;
}

void updateCamera() {
    // Kamera folgt dem Spieler und bleibt in Karten-Grenzen.
    const TileMap& map = g.core.getMap();
    const Player& player = g.core.getPlayer();

    float mapW = static_cast<float>(map.width * 16);
    float mapH = static_cast<float>(map.height * 16);
    float camTargetX = player.x + player.w * 0.5f - 200.0f;
    float camTargetY = player.y + player.h * 0.5f - 120.0f;

    g.camX = clampf(camTargetX, 0.0f, std::max(0.0f, mapW - 400.0f));
    g.camY = clampf(camTargetY, 0.0f, std::max(0.0f, mapH - 240.0f));
}

bool loadLevel(const std::string& level, int originGX, int originGY, float spawnX, float spawnY, bool setSpawn) {
    // Lädt einen Raum und setzt Position/Spawn je nach Übergangskontext.
    std::string mapPath = g.mapsRoot + "/" + level + ".json";
    if (!g.core.loadMapJson(mapPath.c_str())) return false;

    g.currentLevelName = level;
    g.gridX = originGX;
    g.gridY = originGY;

    if (setSpawn) {
        g.core.setPlayerStart(spawnX, spawnY);
    } else {
        g.core.setPlayerPosition(spawnX, spawnY);
    }

    g.currentGridX = g.gridX + static_cast<int>(std::floor(g.core.getPlayer().x / 400.0f));
    g.currentGridY = g.gridY + static_cast<int>(std::floor(g.core.getPlayer().y / 240.0f));

    // Items der gerade geladenen Karte aufbereiten
    g.mapItems.clear();
    const float tileSize = 16.0f;
    for (const auto& d : g.core.getMap().items) {
        // Falls bereits gesammelt, überspringen
        if (d.type == items::double_jump::id() && (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP)) {
            continue;
        }
        App::MapItem mi;
        mi.type = d.type;
        mi.x = d.x * tileSize;
        mi.y = d.y * tileSize;
        mi.collected = false;
        g.mapItems.push_back(mi);
    }

    rebuildMapCache();
    g.visitedCells.insert(makeCellKey(g.currentGridX, g.currentGridY));
    writeVisitedToDisk(g.activeSaveSlot);
    updateCamera();
    return true;
}

bool loadInitialWorldAndMap() {
    // Initialisierung von Welt + Startmap beim Spielstart.
    std::string worldPath = g.mapsRoot + "/world.json";
    if (!g.world.loadWorldJson(worldPath.c_str())) {
        return false;
    }
    g.world.buildSpatialMap(g.mapsRoot.c_str(), 16, 400, 240);

    const auto& cells = g.world.getCells();
    if (cells.empty()) return false;

    int startGX = cells[0].x;
    int startGY = cells[0].y;
    std::string level = cells[0].level;

    for (const auto& c : cells) {
        if (c.level == "0") {
            startGX = c.x;
            startGY = c.y;
            level = c.level;
            break;
        }
    }

    if (!g.core.loadMapJson((g.mapsRoot + "/" + level + ".json").c_str())) {
        return false;
    }

    g.gridX = startGX;
    g.gridY = startGY;
    g.currentLevelName = level;

    if (!g.core.setPlayerStartToFirstEmpty(16.0f)) {
        g.core.setPlayerStart(40.0f, 40.0f);
    }

    const Player& p = g.core.getPlayer();
    g.checkpoint.valid = true;
    g.checkpoint.level = level;
    g.checkpoint.gridX = startGX;
    g.checkpoint.gridY = startGY;
    g.checkpoint.x = p.x;
    g.checkpoint.y = p.y;

    g.visitedCells.clear();
    g.visitedCells.insert(makeCellKey(startGX, startGY));
    rebuildMapCache();
    updateCamera();
    return true;
}

bool startNewGame(int slot) {
    // Startet einen frischen Slot: alte Daten löschen, Startwelt neu laden.
    g.activeSaveSlot = std::max(1, std::min(slot, MainMenuController::SAVE_SLOT_COUNT));
    g.checkpoint = {};
    g.persistentSave = {};
    g.requestMenu = false;
    g.inTransition = false;
    g.lastCheckpointGridValid = false;
    // Itemzustand zurücksetzen
    g.collectedItems = 0;
    g.activeItems = 0;
    g.inventorySelection = 0;
    g.pickupMessage.clear();
    g.pickupMessageTimer = 0.0f;

    std::remove(savePathForSlot(g.activeSaveSlot).c_str());
    resetVisitedProgress(g.activeSaveSlot);

    if (!loadInitialWorldAndMap()) return false;
    writeVisitedToDisk(g.activeSaveSlot);
    return true;
}

bool loadFromCheckpoint(int slot) {
    // Lädt den letzten gespeicherten Checkpoint eines Slots.
    g.activeSaveSlot = std::max(1, std::min(slot, MainMenuController::SAVE_SLOT_COUNT));

    PersistentSave loaded{};
    if (!loadPersistentSaveFromDisk(g.activeSaveSlot, loaded)) return false;
    if (!loaded.valid || loaded.level.empty()) return false;

    // übernehme Itemflags
    g.collectedItems = loaded.collectedItems;
    g.activeItems = loaded.activeItems;
    if (g.activeItems & GameplayScene::ITEM_DOUBLE_JUMP) {
        g.core.getPlayer().hasDoubleJump = true;
    } else {
        g.core.getPlayer().hasDoubleJump = false;
    }

    if (!loadLevel(loaded.level, loaded.gridX, loaded.gridY, loaded.x, loaded.y, false)) {
        return false;
    }

    g.persistentSave = loaded;
    g.checkpoint.valid = true;
    g.checkpoint.level = loaded.level;
    g.checkpoint.gridX = loaded.gridX;
    g.checkpoint.gridY = loaded.gridY;
    g.checkpoint.x = loaded.x;
    g.checkpoint.y = loaded.y;

    g.lastCheckpointGridValid = true;
    g.lastCheckpointGridX = g.currentGridX;
    g.lastCheckpointGridY = g.currentGridY;

    loadVisitedFromDisk(g.activeSaveSlot);
    return true;
}

void handleTransitionIfNeeded() {
    // Prüft Raumübergangs-Tiles und führt bei Treffer den Kartenwechsel aus.
    const auto& transitions = g.core.getMap().getTransitions();
    const Player& player = g.core.getPlayer();

    bool triggered = false;
    Rect hit{};
    for (const auto& tr : transitions) {
        if (player.x < tr.x + tr.w && player.x + player.w > tr.x && player.y < tr.y + tr.h && player.y + player.h > tr.y) {
            triggered = true;
            hit = tr;
            break;
        }
    }

    if (!triggered) {
        g.inTransition = false;
        return;
    }

    if (g.inTransition) return;
    if (!g.world.getCell(g.gridX, g.gridY)) return;
    g.inTransition = true;

    float txCenter = hit.x + hit.w * 0.5f;
    float tyCenter = hit.y + hit.h * 0.5f;
    std::string dir;

    if (txCenter > g.core.getMap().width * 16 - 32) dir = "right";
    else if (txCenter < 32) dir = "left";
    else if (tyCenter > g.core.getMap().height * 16 - 32) dir = "down";
    else if (tyCenter < 32) dir = "up";

    if (dir.empty()) return;

    float targetGX = g.gridX * 400.0f + player.x;
    float targetGY = g.gridY * 240.0f + player.y;
    if (dir == "right") targetGX = (g.gridX * 400.0f) + g.core.getMap().width * 16 + 16;
    else if (dir == "left") targetGX = (g.gridX * 400.0f) - 16;
    else if (dir == "down") targetGY = (g.gridY * 240.0f) + g.core.getMap().height * 16 + 16;
    else if (dir == "up") targetGY = (g.gridY * 240.0f) - 16;

    int nextGX = static_cast<int>(std::floor(targetGX / 400.0f));
    int nextGY = static_cast<int>(std::floor(targetGY / 240.0f));
    const SpatialCell* nextCell = g.world.getCell(nextGX, nextGY);
    if (!nextCell) return;

    std::string levelPath = g.mapsRoot + "/" + nextCell->level + ".json";
    if (!g.core.loadMapJson(levelPath.c_str())) return;
    // mapItems auffrischen (gleiche Logik wie in loadLevel)
    g.mapItems.clear();
    const float tileSize = 16.0f;
    for (const auto& d : g.core.getMap().items) {
        if (d.type == items::double_jump::id() && (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP)) continue;
        App::MapItem mi;
        mi.type = d.type;
        mi.x = d.x * tileSize;
        mi.y = d.y * tileSize;
        mi.collected = false;
        g.mapItems.push_back(mi);
    }
    rebuildMapCache();

    g.gridX = nextCell->originX;
    g.gridY = nextCell->originY;
    g.currentLevelName = nextCell->level;

    float offset = 24.0f;
    float localTargetX = 0.0f;
    float localTargetY = 0.0f;

    if (dir == "right") {
        localTargetX = offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - g.gridY) * 240.0f + screenRelY;
    } else if (dir == "left") {
        float segmentLocalX = (nextGX - g.gridX) * 400.0f;
        localTargetX = segmentLocalX + 400.0f - player.w - offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - g.gridY) * 240.0f + screenRelY;
    } else if (dir == "down") {
        localTargetY = offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - g.gridX) * 400.0f + screenRelX;
    } else {
        float segmentLocalY = (nextGY - g.gridY) * 240.0f;
        localTargetY = segmentLocalY + 240.0f - player.h - offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - g.gridX) * 400.0f + screenRelX;
    }

    // clamp the localTargetX/Y to map bounds before updating state
    {
        const TileMap& m = g.core.getMap();
        float maxX = m.width * 16.0f - g.core.getPlayer().w;
        float maxY = m.height * 16.0f - g.core.getPlayer().h;
        localTargetX = clampf(localTargetX, 0.0f, std::max(0.0f, maxX));
        localTargetY = clampf(localTargetY, 0.0f, std::max(0.0f, maxY));
    }

    if (dir == "right") {
        localTargetX = offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - g.gridY) * 240.0f + screenRelY;
    } else if (dir == "left") {
        float segmentLocalX = (nextGX - g.gridX) * 400.0f;
        localTargetX = segmentLocalX + 400.0f - player.w - offset;
        float screenRelY = std::fmod(player.y, 240.0f);
        localTargetY = (std::floor(targetGY / 240.0f) - g.gridY) * 240.0f + screenRelY;
    } else if (dir == "down") {
        localTargetY = offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - g.gridX) * 400.0f + screenRelX;
    } else {
        float segmentLocalY = (nextGY - g.gridY) * 240.0f;
        localTargetY = segmentLocalY + 240.0f - player.h - offset;
        float screenRelX = std::fmod(player.x, 400.0f);
        localTargetX = (std::floor(targetGX / 400.0f) - g.gridX) * 400.0f + screenRelX;
    }

    if (g.world.isCheckpoint(nextGX, nextGY)) {
        g.checkpoint.valid = true;
        g.checkpoint.level = nextCell->level;
        g.checkpoint.gridX = nextCell->originX;
        g.checkpoint.gridY = nextCell->originY;
        g.checkpoint.x = localTargetX;
        g.checkpoint.y = localTargetY;
        g.core.setPlayerStart(localTargetX, localTargetY);
        g.lastCheckpointGridValid = true;
        g.lastCheckpointGridX = nextGX;
        g.lastCheckpointGridY = nextGY;
        writePersistentSaveToDisk(g.checkpoint, g.activeSaveSlot);
    } else {
        g.core.setPlayerPosition(localTargetX, localTargetY);
    }

    g.currentGridX = g.gridX + static_cast<int>(std::floor(g.core.getPlayer().x / 400.0f));
    g.currentGridY = g.gridY + static_cast<int>(std::floor(g.core.getPlayer().y / 240.0f));
    if (g.visitedCells.insert(makeCellKey(g.currentGridX, g.currentGridY)).second) {
        writeVisitedToDisk(g.activeSaveSlot);
    }
}

void updateGameplay(float dt) {
    // Zentrales Gameplay-Update: Input -> Physik -> Transition -> Respawn -> FPS.
    InputState input;
    input.left = g.moveLeftHeld;
    input.right = g.moveRightHeld;
    input.jump = g.jumpHeld;
    input.jumpPressed = g.jumpPressed;
    g.jumpPressed = false;

    g.core.update(input, dt);

    const Player& p = g.core.getPlayer();
    g.currentGridX = g.gridX + static_cast<int>(std::floor(p.x / 400.0f));
    g.currentGridY = g.gridY + static_cast<int>(std::floor(p.y / 240.0f));

    // Item-Kollision prüfen
    const float itemSize = 16.0f;
    for (auto& it : g.mapItems) {
        if (it.collected) continue;
        if (p.x < it.x + itemSize && p.x + p.w > it.x && p.y < it.y + itemSize && p.y + p.h > it.y) {
            it.collected = true;
            if (it.type == items::double_jump::id()) {
                g.collectedItems |= GameplayScene::ITEM_DOUBLE_JUMP;
                g.activeItems |= GameplayScene::ITEM_DOUBLE_JUMP;
                g.core.getPlayer().hasDoubleJump = true;
                g.pickupMessage = "Doppelsprung erhalten";
                g.pickupMessageTimer = 2.0f;
            }
            // cartesische Darstellung aktualisieren (Cache neu bauen)
            rebuildMapCache();
        }
    }

    updateCheckpointFromCurrentCell();

    if (g.world.getCell(g.currentGridX, g.currentGridY)) {
        if (g.visitedCells.insert(makeCellKey(g.currentGridX, g.currentGridY)).second) {
            writeVisitedToDisk(g.activeSaveSlot);
        }
    }

    handleTransitionIfNeeded();

    if (g.core.consumeDeath()) {
        if (g.checkpoint.valid && !g.checkpoint.level.empty()) {
            loadLevel(g.checkpoint.level, g.checkpoint.gridX, g.checkpoint.gridY, g.checkpoint.x, g.checkpoint.y, false);
        }
    }

    updateCamera();

    g.fpsFrameCounter++;
    u64 t = nowMs();
    if (t - g.fpsTimerMs >= 1000) {
        g.fpsValue = g.fpsFrameCounter;
        g.fpsFrameCounter = 0;
        g.fpsTimerMs = t;
    }
    // Message-Timer für Items verringern
    if (g.pickupMessageTimer > 0.0f) {
        g.pickupMessageTimer -= dt;
        if (g.pickupMessageTimer < 0.0f) g.pickupMessageTimer = 0.0f;
    }
}

bool inRect(const RectI& r, int x, int y) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

void queueTouchFromMouse(int screenX, int screenY) {
    if (!inRect(g.bottomView, screenX, screenY)) return;
    int localX = screenX - g.bottomView.x;
    int localY = screenY - g.bottomView.y;
    localX = std::max(0, std::min(319, localX));
    localY = std::max(0, std::min(239, localY));
    g.pendingTouch.px = static_cast<std::uint16_t>(localX);
    g.pendingTouch.py = static_cast<std::uint16_t>(localY);
    g.touchPending = true;
}

void handleMenuLogic() {
    // Verarbeitet Menüeingaben und ausgelöste Aktionen.
    u32 kDown = g.keyDown;
    g.keyDown = 0;

    int selectedSlot = g.menu.getSelectedSaveSlot();
    g.menu.setHasContinue(hasPersistentSave(selectedSlot));
    g.menu.handleKeys(kDown);

    if (g.touchPending) {
        g.menu.handleTouch(g.pendingTouch);
        g.touchPending = false;
    }

    MainMenuAction action = g.menu.consumeAction();
    if (action.startGame) {
        if (startNewGame(g.menu.getSelectedSaveSlot())) {
            g.appState = APP_GAME;
            g.requestMenu = false;
        }
    }
    if (action.loadCheckpoint) {
        if (loadFromCheckpoint(g.menu.getSelectedSaveSlot())) {
            g.appState = APP_GAME;
            g.requestMenu = false;
        }
    }
    if (action.resetMapProgress) {
        resetVisitedProgress(g.menu.getSelectedSaveSlot());
    }
    if (action.exitGame) {
        PostQuitMessage(0);
    }

    g.controlsSwapped = g.menu.getControlsSwapped();
}

void handleGameplayInput() {
    // Übersetzt Keybits/Touch in Bewegungs-, Tab- und Menüaktionen.
    const u32 kDown = g.keyDown;
    const u32 kHeld = g.keyHeld;
    g.keyDown = 0;

    const u32 topLeft = g.controlsSwapped ? KEY_DLEFT : KEY_CPAD_LEFT;
    const u32 topRight = g.controlsSwapped ? KEY_DRIGHT : KEY_CPAD_RIGHT;
    const u32 topJump = KEY_A;

    const u32 bottomLeft = g.controlsSwapped ? KEY_CPAD_LEFT : KEY_DLEFT;
    const u32 bottomRight = g.controlsSwapped ? KEY_CPAD_RIGHT : KEY_DRIGHT;
    const u32 bottomUp = g.controlsSwapped ? KEY_CPAD_UP : KEY_DUP;
    const u32 bottomDown = g.controlsSwapped ? KEY_CPAD_DOWN : KEY_DDOWN;

    g.moveLeftHeld = (kHeld & topLeft) != 0;
    g.moveRightHeld = (kHeld & topRight) != 0;
    g.jumpHeld = (kHeld & topJump) != 0;
    g.jumpPressed = (kDown & topJump) != 0;

    if (kDown & KEY_SELECT) {
        g.requestMenu = true;
        return;
    }

    if (kDown & bottomLeft) g.bottomMode = (g.bottomMode + 3) % 4;
    if (kDown & bottomRight) g.bottomMode = (g.bottomMode + 1) % 4;

    if (g.touchPending) {
        const touchPosition tp = g.pendingTouch;

        if (tp.py >= 220) {
            g.bottomMode = tp.px / 80;
            if (g.bottomMode < 0) g.bottomMode = 0;
            if (g.bottomMode > 3) g.bottomMode = 3;
            g.touchPending = false;
        } else if (g.bottomMode == TAB_SETTINGS) {
            if (tp.py >= 54 && tp.py <= 96) {
                g.settingsSelection = 0;
                g.showFpsEnabled = !g.showFpsEnabled;
            } else if (tp.py >= 104 && tp.py <= 146) {
                g.settingsSelection = 1;
                g.requestMenu = true;
            }
            g.touchPending = false;
        } else if (g.bottomMode == TAB_INVENTORY) {
            // handle tap on item row
            int count = 0;
            if (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP) count++;
            if (count > 0) {
                if (tp.px >= 16 && tp.px < 304 && tp.py >= 54) {
                    int row = (tp.py - 54) / 24;
                    if (row == g.inventorySelection) {
                        // toggle current entry
                        if (g.inventorySelection == 0 && (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP)) {
                            if (g.activeItems & GameplayScene::ITEM_DOUBLE_JUMP) {
                                g.activeItems &= ~GameplayScene::ITEM_DOUBLE_JUMP;
                                g.core.getPlayer().hasDoubleJump = false;
                            } else {
                                g.activeItems |= GameplayScene::ITEM_DOUBLE_JUMP;
                                g.core.getPlayer().hasDoubleJump = true;
                            }
                        }
                    } else if (row >= 0 && row < count) {
                        g.inventorySelection = row;
                    }
                }
            }
            g.touchPending = false;
        }
    }

    if (g.bottomMode == TAB_INVENTORY) {
        // Navigation durch gesammelte Items
        int count = 0;
        if (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP) count++;
        if (count > 0) {
            if (kDown & bottomUp) g.inventorySelection = std::max(0, g.inventorySelection - 1);
            if (kDown & bottomDown) g.inventorySelection = std::min(count - 1, g.inventorySelection + 1);
            if (kDown & KEY_Y) {
                if (g.inventorySelection == 0 && (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP)) {
                    if (g.activeItems & GameplayScene::ITEM_DOUBLE_JUMP) {
                        g.activeItems &= ~GameplayScene::ITEM_DOUBLE_JUMP;
                        g.core.getPlayer().hasDoubleJump = false;
                    } else {
                        g.activeItems |= GameplayScene::ITEM_DOUBLE_JUMP;
                        g.core.getPlayer().hasDoubleJump = true;
                    }
                }
            }
        }
    } else if (g.bottomMode == TAB_SETTINGS) {
        if (kDown & bottomUp) g.settingsSelection = (g.settingsSelection + 1) % 2;
        if (kDown & bottomDown) g.settingsSelection = (g.settingsSelection + 1) % 2;
        if (kDown & KEY_Y) {
            if (g.settingsSelection == 0) g.showFpsEnabled = !g.showFpsEnabled;
            else g.requestMenu = true;
        }
    } else if (g.bottomMode == TAB_DEBUG) {
        if (kDown & bottomUp) g.debugScrollPx -= 18;
        if (kDown & bottomDown) g.debugScrollPx += 18;

        if (g.touchPending) {
            const touchPosition tp = g.pendingTouch;
            g.touchPending = false;
            if (tp.px >= 294 && tp.py >= 44 && tp.py <= 196 && g.debugMaxScrollPx > 0) {
                float t = (tp.py - 44.0f) / (196.0f - 44.0f);
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                g.debugScrollPx = static_cast<int>(t * g.debugMaxScrollPx);
            }
        }

        if (g.debugScrollPx < 0) g.debugScrollPx = 0;
        if (g.debugScrollPx > g.debugMaxScrollPx) g.debugScrollPx = g.debugMaxScrollPx;
    }

    if (g.touchPending) {
        g.touchPending = false;
    }
}

void mapVKeyToKeys(UINT vk, bool down) {
    // Mappt Tastatur-Events auf 3DS-nahe Input-Bits.
    auto setBit = [down](u32& mask, u32 bit) {
        if (down) mask |= bit;
        else mask &= ~bit;
    };

    auto pressEdge = [down](u32& keyDownMask, u32 bit) {
        if (down) keyDownMask |= bit;
    };

    u32 bits = 0;
    switch (vk) {
        case VK_LEFT:
            bits = KEY_CPAD_LEFT;
            break;
        case VK_RIGHT:
            bits = KEY_CPAD_RIGHT;
            break;
        case VK_UP:
            bits = KEY_CPAD_UP;
            break;
        case VK_DOWN:
            bits = KEY_CPAD_DOWN;
            break;

        // Bottom-Screen/DPad-Steuerung am Desktop
        case 'A': bits = KEY_DLEFT; break;
        case 'D': bits = KEY_DRIGHT; break;
        case 'W': bits = KEY_DUP; break;
        case 'S': bits = KEY_DDOWN; break;

        case VK_SPACE: bits = KEY_A; break;
        case VK_RETURN: bits = KEY_A | KEY_Y; break;
        case VK_BACK: bits = KEY_B; break;
        case VK_ESCAPE: bits = KEY_SELECT; break;
        case 'Q': bits = KEY_DLEFT; break;
        case 'E': bits = KEY_DRIGHT; break;
        case 'I': bits = KEY_DUP; break;
        case 'K': bits = KEY_DDOWN; break;
        case 'R': bits = KEY_R; break;
        case 'Y': bits = KEY_Y; break; // allow Y key to toggle inventory
        default: break;
    }

    if (bits == 0) return;

    for (int i = 0; i < 32; ++i) {
        u32 bit = 1u << i;
        if ((bits & bit) == 0) continue;
        if (down && (g.keyHeld & bit) == 0) {
            pressEdge(g.keyDown, bit);
        }
        setBit(g.keyHeld, bit);
    }
}

void fillRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT r{x, y, x + w, y + h};
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, color);
    FillRect(hdc, &r, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    SelectObject(hdc, oldBrush);
}

void drawText(HDC hdc, int x, int y, COLORREF color, const char* text) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    TextOutA(hdc, x, y, text, static_cast<int>(std::strlen(text)));
}

void drawTopGameplay(HDC hdc) {
    // Zeichnet Top-Screen-Inhalte im Gameplay (Welt, Spieler, Debug/FPS).
    fillRect(hdc, g.topView.x, g.topView.y, g.topView.w, g.topView.h, RGB(16, 20, 32));

    const TileMap& map = g.core.getMap();
    const Player& p = g.core.getPlayer();

    if (g.mapDc && g.mapBmp) {
        int srcX = static_cast<int>(g.camX);
        int srcY = static_cast<int>(g.camY);
        if (srcX < 0) srcX = 0;
        if (srcY < 0) srcY = 0;
        BitBlt(hdc, g.topView.x, g.topView.y, 400, 240, g.mapDc, srcX, srcY, SRCCOPY);
    } else {
        int tx0 = static_cast<int>(g.camX / 16.0f);
        int ty0 = static_cast<int>(g.camY / 16.0f);
        int tx1 = tx0 + 26;
        int ty1 = ty0 + 16;
        for (int ty = ty0; ty <= ty1; ++ty) {
            for (int tx = tx0; tx <= tx1; ++tx) {
                int tile = map.getTile(tx, ty);
                if (tile <= 0) continue;
                int sx = g.topView.x + static_cast<int>(tx * 16 - g.camX);
                int sy = g.topView.y + static_cast<int>(ty * 16 - g.camY);
                fillRect(hdc, sx, sy, 16, 16, colorForTile(tile));
            }
        }
    }

    fillRect(hdc,
             g.topView.x + static_cast<int>(p.x - g.camX),
             g.topView.y + static_cast<int>(p.y - g.camY),
             static_cast<int>(p.w),
             static_cast<int>(p.h),
             RGB(250, 210, 60));

    for (const auto& tr : map.getTransitions()) {
        int sx = g.topView.x + static_cast<int>(tr.x - g.camX);
        int sy = g.topView.y + static_cast<int>(tr.y - g.camY);
        fillRect(hdc, sx, sy, static_cast<int>(tr.w), static_cast<int>(tr.h), RGB(0, 220, 0));
    }

    if (g.showFpsEnabled) {
        char fpsText[64] = {0};
        std::snprintf(fpsText, sizeof(fpsText), "FPS: %d", g.fpsValue);
        drawText(hdc, g.topView.x + 6, g.topView.y + 6, RGB(130, 255, 150), fpsText);
        if (g.pickupMessageTimer > 0.0f && !g.pickupMessage.empty()) {
            drawText(hdc, g.topView.x + 200, g.topView.y + 20, RGB(240, 220, 120), g.pickupMessage.c_str());
        }
    }

    if (g.menu.getDebugEnabled()) {
        int playerTileX = static_cast<int>(std::floor(p.x / 16.0f));
        int playerTileY = static_cast<int>(std::floor(p.y / 16.0f));
        int playerTileId = map.getTile(playerTileX, playerTileY);

        char line1[160] = {0};
        char line2[160] = {0};
        char line3[160] = {0};
        std::snprintf(line1, sizeof(line1), "Lvl:%s Grid:%d,%d O:%d,%d", g.currentLevelName.c_str(), g.currentGridX, g.currentGridY, g.gridX, g.gridY);
        std::snprintf(line2, sizeof(line2), "Pos:%.1f/%.1f Vel:%.1f/%.1f", p.x, p.y, p.vx, p.vy);
        std::snprintf(line3, sizeof(line3), "Tile:%d,%d id:%d G:%s T:%s", playerTileX, playerTileY, playerTileId, p.grounded ? "Y" : "N", g.inTransition ? "Y" : "N");
        drawText(hdc, g.topView.x + 6, g.topView.y + 22, RGB(180, 210, 255), line1);
        drawText(hdc, g.topView.x + 6, g.topView.y + 36, RGB(180, 210, 255), line2);
        drawText(hdc, g.topView.x + 6, g.topView.y + 50, RGB(180, 210, 255), line3);
    }
}

void drawBottomTabs(HDC hdc) {
    static const char* labels[4] = {"Map", "Inv", "Set", "Dbg"};
    fillRect(hdc, g.bottomView.x, g.bottomView.y + 220, 320, 20, RGB(30, 30, 45));
    for (int i = 0; i < 4; ++i) {
        COLORREF col = (i == g.bottomMode) ? RGB(80, 140, 220) : RGB(50, 50, 70);
        fillRect(hdc, g.bottomView.x + i * 80, g.bottomView.y + 220, 80, 20, col);
        drawText(hdc, g.bottomView.x + 28 + i * 80, g.bottomView.y + 223, RGB(235, 235, 245), labels[i]);
    }
}

void drawBottomGameplay(HDC hdc) {
    // Zeichnet Bottom-Tabs und deren Inhalte (Map/Inv/Settings/Debug).
    fillRect(hdc, g.bottomView.x, g.bottomView.y, g.bottomView.w, g.bottomView.h, RGB(20, 20, 30));
    fillRect(hdc, g.bottomView.x + 6, g.bottomView.y + 8, 308, 208, RGB(18, 24, 34));
    g.debugMaxScrollPx = 0;

    if (g.bottomMode == TAB_MAP) {
        drawText(hdc, g.bottomView.x + 12, g.bottomView.y + 12, RGB(230, 230, 240), "Karte");
        char gridText[64] = {0};
        std::snprintf(gridText, sizeof(gridText), "GX:%d GY:%d", g.currentGridX, g.currentGridY);
        drawText(hdc, g.bottomView.x + 214, g.bottomView.y + 12, RGB(170, 180, 205), gridText);

        int minX = 0, minY = 0, maxX = 0, maxY = 0;
        if (g.world.getSpatialBounds(minX, minY, maxX, maxY)) {
            int cols = (maxX - minX) + 1;
            int rows = (maxY - minY) + 1;
            int panelX = g.bottomView.x + 12;
            int panelY = g.bottomView.y + 28;
            int panelW = 296;
            int panelH = 180;
            int cellW = panelW / (cols > 0 ? cols : 1);
            int cellH = panelH / (rows > 0 ? rows : 1);
            int cell = cellW < cellH ? cellW : cellH;
            if (cell < 6) cell = 6;
            if (cell > 20) cell = 20;

            int drawW = cols * cell;
            int drawH = rows * cell;
            int ox = panelX + (panelW - drawW) / 2;
            int oy = panelY + (panelH - drawH) / 2;
            fillRect(hdc, ox - 2, oy - 2, drawW + 4, drawH + 4, RGB(36, 46, 62));

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    const SpatialCell* sc = g.world.getCell(x, y);
                    if (!sc) continue;
                    bool isCurrent = (x == g.currentGridX && y == g.currentGridY);
                    bool isVisited = g.visitedCells.find(makeCellKey(x, y)) != g.visitedCells.end();
                    if (!isVisited && !isCurrent) continue;
                    bool isOrigin = (x == sc->originX && y == sc->originY);
                    COLORREF col = isCurrent ? RGB(100, 220, 130) : (isOrigin ? RGB(102, 150, 210) : RGB(76, 112, 164));
                    fillRect(hdc, ox + (x - minX) * cell, oy + (y - minY) * cell, cell - 1, cell - 1, col);
                }
            }
        }
    } else if (g.bottomMode == TAB_INVENTORY) {
        drawText(hdc, g.bottomView.x + 12, g.bottomView.y + 12, RGB(230, 230, 240), "Inventar");
        fillRect(hdc, g.bottomView.x + 16, g.bottomView.y + 44, 288, 144, RGB(28, 34, 46));
        // Liste der gesammelten Items zeichnen
        int drawY = g.bottomView.y + 54;
        int idx = 0;
        if (g.collectedItems & GameplayScene::ITEM_DOUBLE_JUMP) {
            bool active = (g.activeItems & GameplayScene::ITEM_DOUBLE_JUMP) != 0;
            bool sel = (g.inventorySelection == idx);
            if (sel) {
                fillRect(hdc, g.bottomView.x + 16, drawY - 2, 288, 20, RGB(50,50,70));
            }
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Doppelsprung: %s", active ? "ON" : "OFF");
            drawText(hdc, g.bottomView.x + 24, drawY, RGB(235, 235, 245), buf);
            drawY += 24;
            idx++;
        }
        if (idx == 0) {
            drawText(hdc, g.bottomView.x + 44, g.bottomView.y + 108, RGB(170, 180, 205), "(Aktuell leer)");
        }
    } else if (g.bottomMode == TAB_SETTINGS) {
        drawText(hdc, g.bottomView.x + 12, g.bottomView.y + 12, RGB(230, 230, 240), "Einstellungen");
        COLORREF c0 = (g.settingsSelection == 0) ? RGB(76, 112, 182) : RGB(46, 62, 92);
        COLORREF c1 = (g.settingsSelection == 1) ? RGB(182, 84, 84) : RGB(92, 46, 46);
        fillRect(hdc, g.bottomView.x + 20, g.bottomView.y + 54, 280, 42, c0);
        fillRect(hdc, g.bottomView.x + 20, g.bottomView.y + 104, 280, 42, c1);
        drawText(hdc, g.bottomView.x + 28, g.bottomView.y + 67, RGB(236, 236, 245), "FPS Anzeige");
        drawText(hdc, g.bottomView.x + 248, g.bottomView.y + 67, g.showFpsEnabled ? RGB(130, 250, 160) : RGB(255, 180, 180), g.showFpsEnabled ? "AN" : "AUS");
        drawText(hdc, g.bottomView.x + 28, g.bottomView.y + 117, RGB(245, 236, 236), "Zum Hauptmenue");
    } else {
        drawText(hdc, g.bottomView.x + 12, g.bottomView.y + 12, RGB(230, 230, 240), "Debug");
        fillRect(hdc, g.bottomView.x + 16, g.bottomView.y + 38, 288, 166, RGB(24, 30, 42));
        if (!g.menu.getDebugEnabled()) {
            drawText(hdc, g.bottomView.x + 26, g.bottomView.y + 110, RGB(180, 190, 210), "Debug ist im Hauptmenue AUS");
        } else {
            const Player& p = g.core.getPlayer();
            const TileMap& map = g.core.getMap();
            int playerTileX = static_cast<int>(std::floor(p.x / 16.0f));
            int playerTileY = static_cast<int>(std::floor(p.y / 16.0f));
            int playerTileId = map.getTile(playerTileX, playerTileY);

            int minX = 0;
            int minY = 0;
            int maxX = 0;
            int maxY = 0;
            bool hasBounds = g.world.getSpatialBounds(minX, minY, maxX, maxY);
            int transitions = static_cast<int>(map.getTransitions().size());
            int worldCells = static_cast<int>(g.world.getCells().size());
            int mapPixelsW = map.width * 16;
            int mapPixelsH = map.height * 16;

            const float viewportTop = 44.0f;
            const float viewportBottom = 196.0f;
            const float lineHeight = 18.0f;
            const int lineCount = 13;
            const int contentHeight = static_cast<int>(lineCount * lineHeight);
            const int viewportHeight = static_cast<int>(viewportBottom - viewportTop);
            int maxScroll = contentHeight - viewportHeight;
            if (maxScroll < 0) maxScroll = 0;
            g.debugMaxScrollPx = maxScroll;
            if (g.debugScrollPx < 0) g.debugScrollPx = 0;
            if (g.debugScrollPx > g.debugMaxScrollPx) g.debugScrollPx = g.debugMaxScrollPx;

            auto drawIfVisible = [&](float y, const char* fmt, auto... vals) {
                if (y < viewportTop || y > viewportBottom) return;
                char line[192] = {0};
                std::snprintf(line, sizeof(line), fmt, vals...);
                drawText(hdc, g.bottomView.x + 24, g.bottomView.y + static_cast<int>(y), RGB(190, 210, 235), line);
            };

            float y = 44.0f - static_cast<float>(g.debugScrollPx);
            drawIfVisible(y, "Lvl:%s  FPS:%d", g.currentLevelName.c_str(), g.fpsValue); y += lineHeight;
            drawIfVisible(y, "Map Tiles:%dx%d  Px:%dx%d", map.width, map.height, mapPixelsW, mapPixelsH); y += lineHeight;
            drawIfVisible(y, "Grid Cur:%d,%d  Origin:%d,%d", g.currentGridX, g.currentGridY, g.gridX, g.gridY); y += lineHeight;
            drawIfVisible(y, "World Cells:%d  Bounds:%s", worldCells, hasBounds ? "OK" : "NONE"); y += lineHeight;
            if (hasBounds) {
                drawIfVisible(y, "Bound Min:%d,%d  Max:%d,%d", minX, minY, maxX, maxY);
            } else {
                drawIfVisible(y, "Bound Min/Max: -");
            }
            y += lineHeight;
            drawIfVisible(y, "Player Pos:%.1f / %.1f", p.x, p.y); y += lineHeight;
            drawIfVisible(y, "Player Vel:%.1f / %.1f", p.vx, p.vy); y += lineHeight;
            drawIfVisible(y, "Player Size:%.1f x %.1f", p.w, p.h); y += lineHeight;
            drawIfVisible(y, "Tile X/Y:%d/%d  ID:%d", playerTileX, playerTileY, playerTileId); y += lineHeight;
            drawIfVisible(y, "Cam X/Y:%.1f / %.1f", g.camX, g.camY); y += lineHeight;
            drawIfVisible(y, "Ground:%s  Coyote:%.2f", p.grounded ? "Y" : "N", p.coyoteTimer); y += lineHeight;
            drawIfVisible(y, "Transition:%s  Count:%d", g.inTransition ? "Y" : "N", transitions); y += lineHeight;
            drawIfVisible(y, "Checkpoint:%s (%d,%d)", g.checkpoint.valid ? "Y" : "N", g.checkpoint.gridX, g.checkpoint.gridY);

            if (maxScroll > 0) {
                fillRect(hdc, g.bottomView.x + 296, g.bottomView.y + 44, 4, 152, RGB(58, 72, 96));
                float trackH = 152.0f;
                float thumbH = (trackH * trackH) / static_cast<float>(contentHeight);
                if (thumbH < 18.0f) thumbH = 18.0f;
                float t = static_cast<float>(g.debugScrollPx) / static_cast<float>(maxScroll);
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                float thumbY = 44.0f + t * (trackH - thumbH);
                fillRect(hdc, g.bottomView.x + 296, g.bottomView.y + static_cast<int>(thumbY), 4, static_cast<int>(thumbH), RGB(132, 176, 236));
            }
        }
    }

    drawBottomTabs(hdc);
}

void drawTopMenu(HDC hdc) {
    // Zeichnet Top-Screen des Hauptmenüs inkl. Kontext/Hinweise.
    fillRect(hdc, g.topView.x, g.topView.y, 400, 240, RGB(16, 20, 32));
    fillRect(hdc, g.topView.x + 16, g.topView.y + 14, 368, 34, RGB(30, 40, 62));
    drawText(hdc, g.topView.x + 26, g.topView.y + 24, RGB(244, 248, 255), "Metroidvania 3DS");
    drawText(hdc, g.topView.x + 250, g.topView.y + 24, RGB(180, 194, 224), "C++ Core Build");

    fillRect(hdc, g.topView.x + 16, g.topView.y + 58, 140, 166, RGB(26, 34, 52));
    fillRect(hdc, g.topView.x + 164, g.topView.y + 58, 220, 166, RGB(32, 42, 64));

    MainMenuState state = g.menu.getState();
    int categorySelection = 0;
    if (state == MENU_HOME) categorySelection = g.menu.getHomeSelection();
    else if (state == MENU_PLAY) categorySelection = 0;
    else if (state == MENU_OPTIONS) categorySelection = 2;
    else categorySelection = 1;

    static const char* categories[4] = {"Spielen", "Handbuch", "Optionen", "Beenden"};
    for (int i = 0; i < 4; ++i) {
        COLORREF rowCol = (i == categorySelection) ? RGB(74, 118, 194) : RGB(42, 56, 84);
        fillRect(hdc, g.topView.x + 24, g.topView.y + 66 + i * 36, 124, 28, rowCol);
        drawText(hdc, g.topView.x + 34, g.topView.y + 74 + i * 36, RGB(238, 242, 250), categories[i]);
    }

    if (state == MENU_MANUAL_LIST || state == MENU_MANUAL_PAGE) {
        int topic = g.menu.getManualSelection();
        if (topic >= manualTopicCount()) topic = manualTopicCount() - 1;
        if (topic < 0) topic = 0;

        if (state == MENU_MANUAL_LIST) {
            drawText(hdc, g.topView.x + 176, g.topView.y + 72, RGB(238, 242, 250), "Handbuch");
            if (g.menu.getManualSelection() < manualTopicCount()) {
                drawText(hdc, g.topView.x + 176, g.topView.y + 100, RGB(208, 220, 244), manualTopicName(topic));
                drawText(hdc, g.topView.x + 176, g.topView.y + 124, RGB(182, 198, 230), "A/Y: Thema oeffnen");
                drawText(hdc, g.topView.x + 176, g.topView.y + 144, RGB(182, 198, 230), "DPad: Thema waehlen");
                drawText(hdc, g.topView.x + 176, g.topView.y + 164, RGB(182, 198, 230), "Inhalte sind scrollbar.");
            } else {
                drawText(hdc, g.topView.x + 176, g.topView.y + 100, RGB(208, 220, 244), "Zurueck");
            }
        } else {
            drawText(hdc, g.topView.x + 176, g.topView.y + 72, RGB(238, 242, 250), manualTopicName(topic));
            int startLine = g.menu.getManualScroll();
            int y = g.topView.y + 94;
            for (int i = 0; i < 12; ++i) {
                const char* line = manualPageLine(topic, startLine + i);
                if (!line || line[0] == '\0') {
                    y += 8;
                } else {
                    COLORREF c = RGB(208, 220, 244);
                    if (std::strstr(line, "ROT ROT = Gefahr") != nullptr) c = RGB(255, 120, 120);
                    if (std::strstr(line, "GRUEN GRUEN = Raumuebergang") != nullptr) c = RGB(140, 235, 150);
                    drawText(hdc, g.topView.x + 176, y, c, line);
                    y += 18;
                }
            }
        }
    } else if (state == MENU_OPTIONS) {
        int sel = g.menu.getOptionsSelection();
        drawText(hdc, g.topView.x + 176, g.topView.y + 72, RGB(238, 242, 250), "Einstellungs-Hilfe");
        if (sel == 0) {
            drawText(hdc, g.topView.x + 176, g.topView.y + 96, RGB(208, 220, 244), "Ingame Debug Info");
            drawText(hdc, g.topView.x + 176, g.topView.y + 118, RGB(182, 198, 230), "Zeigt technische Werte ingame an.");
            drawText(hdc, g.topView.x + 176, g.topView.y + 140, g.menu.getDebugEnabled() ? RGB(136, 240, 170) : RGB(220, 170, 170),
                     g.menu.getDebugEnabled() ? "Aktuell: AN" : "Aktuell: AUS");
        } else if (sel == 1) {
            drawText(hdc, g.topView.x + 176, g.topView.y + 96, RGB(208, 220, 244), "Steuerung tauschen");
            drawText(hdc, g.topView.x + 176, g.topView.y + 118, RGB(182, 198, 230), "Wechselt Top/Bottom Eingabe-Schema.");
            drawText(hdc, g.topView.x + 176, g.topView.y + 140, g.menu.getControlsSwapped() ? RGB(136, 240, 170) : RGB(220, 170, 170),
                     g.menu.getControlsSwapped() ? "Aktuell: TAUSCH" : "Aktuell: STANDARD");
        } else if (sel == 2) {
            drawText(hdc, g.topView.x + 176, g.topView.y + 96, RGB(208, 220, 244), "Kartenfortschritt reset");
            drawText(hdc, g.topView.x + 176, g.topView.y + 118, RGB(182, 198, 230), "Loescht den Fog-of-War des Slots.");
        } else {
            drawText(hdc, g.topView.x + 176, g.topView.y + 96, RGB(208, 220, 244), "Zurueck");
        }
    } else {
        drawText(hdc, g.topView.x + 176, g.topView.y + 72, RGB(238, 242, 250), "Status");
        char slot[64] = {0};
        std::snprintf(slot, sizeof(slot), "Aktiver Slot: %d", g.menu.getSelectedSaveSlot());
        drawText(hdc, g.topView.x + 176, g.topView.y + 96, RGB(190, 206, 236), slot);
        drawText(hdc, g.topView.x + 176, g.topView.y + 120,
                 g.menu.hasContinueAvailable() ? RGB(136, 240, 170) : RGB(220, 170, 170),
                 g.menu.hasContinueAvailable() ? "Fortsetzen verfuegbar" : "Kein Save im Slot");
    }

    drawText(hdc, g.topView.x + 176, g.topView.y + 188, RGB(168, 184, 216), "Bottom: Aktionen / Auswahl");
    drawText(hdc, g.topView.x + 176, g.topView.y + 208, RGB(168, 184, 216), "A/Y: Bestaetigen   B: Zurueck");
}

void drawBottomMenu(HDC hdc) {
    // Zeichnet Bottom-Screen des Hauptmenüs inkl. interaktiver Listen.
    fillRect(hdc, g.bottomView.x, g.bottomView.y, 320, 240, RGB(26, 28, 38));

    MainMenuState state = g.menu.getState();
    if (state == MENU_HOME) {
        static const char* labels[4] = {"Spielen", "Handbuch", "Optionen", "Spiel beenden"};
        for (int i = 0; i < 4; ++i) {
            COLORREF c = (g.menu.getHomeSelection() == i) ? RGB(84, 140, 220)
                                                          : (i == 3 ? RGB(96, 50, 50) : RGB(56, 72, 104));
            fillRect(hdc, g.bottomView.x + 50, g.bottomView.y + 48 + i * 48, 220, 32, c);
            drawText(hdc, g.bottomView.x + 78, g.bottomView.y + 58 + i * 48, RGB(245, 245, 255), labels[i]);
        }
    } else if (state == MENU_PLAY) {
        fillRect(hdc, g.bottomView.x + 44, g.bottomView.y + 8, 40, 28, RGB(56, 72, 104));
        fillRect(hdc, g.bottomView.x + 236, g.bottomView.y + 8, 40, 28, RGB(56, 72, 104));
        drawText(hdc, g.bottomView.x + 58, g.bottomView.y + 16, RGB(245, 245, 255), "<");
        drawText(hdc, g.bottomView.x + 250, g.bottomView.y + 16, RGB(245, 245, 255), ">");

        char slotText[64] = {0};
        std::snprintf(slotText, sizeof(slotText), "Slot %d", g.menu.getSelectedSaveSlot());
        drawText(hdc, g.bottomView.x + 120, g.bottomView.y + 18, RGB(192, 205, 235), slotText);

        COLORREF c0 = g.menu.hasContinueAvailable() ?
            (g.menu.getPlaySelection() == 0 ? RGB(84, 140, 220) : RGB(56, 72, 104)) : RGB(68, 68, 76);
        COLORREF c1 = (g.menu.getPlaySelection() == 1) ? RGB(84, 140, 220) : RGB(56, 72, 104);
        COLORREF c2 = (g.menu.getPlaySelection() == 2) ? RGB(124, 96, 186) : RGB(70, 58, 102);

        fillRect(hdc, g.bottomView.x + 50, g.bottomView.y + 48, 220, 32, c0);
        fillRect(hdc, g.bottomView.x + 50, g.bottomView.y + 96, 220, 32, c1);
        fillRect(hdc, g.bottomView.x + 50, g.bottomView.y + 144, 220, 32, c2);

        drawText(hdc, g.bottomView.x + 72, g.bottomView.y + 58,
                 g.menu.hasContinueAvailable() ? RGB(245, 245, 255) : RGB(170, 170, 182),
                 g.menu.hasContinueAvailable() ? "Fortsetzen" : "Fortsetzen (kein Save)");
        drawText(hdc, g.bottomView.x + 86, g.bottomView.y + 106, RGB(245, 245, 255), "Neues Spiel");
        drawText(hdc, g.bottomView.x + 112, g.bottomView.y + 154, RGB(245, 245, 255), "Zurueck");
    } else if (state == MENU_OPTIONS) {
        COLORREF c0 = (g.menu.getOptionsSelection() == 0) ? RGB(84, 140, 220) : RGB(56, 72, 104);
        COLORREF c1 = (g.menu.getOptionsSelection() == 1) ? RGB(84, 140, 220) : RGB(56, 72, 104);
        COLORREF c2 = (g.menu.getOptionsSelection() == 2) ? RGB(186, 116, 74) : RGB(108, 72, 48);
        COLORREF c3 = (g.menu.getOptionsSelection() == 3) ? RGB(124, 96, 186) : RGB(70, 58, 102);

        fillRect(hdc, g.bottomView.x + 40, g.bottomView.y + 42, 240, 32, c0);
        fillRect(hdc, g.bottomView.x + 40, g.bottomView.y + 90, 240, 32, c1);
        fillRect(hdc, g.bottomView.x + 40, g.bottomView.y + 138, 240, 32, c2);
        fillRect(hdc, g.bottomView.x + 40, g.bottomView.y + 186, 240, 32, c3);

        drawText(hdc, g.bottomView.x + 52, g.bottomView.y + 50, RGB(245, 245, 255), "Ingame Debug Info");
        drawText(hdc, g.bottomView.x + 226, g.bottomView.y + 50,
                 g.menu.getDebugEnabled() ? RGB(120, 250, 160) : RGB(255, 180, 180),
                 g.menu.getDebugEnabled() ? "AN" : "AUS");
        drawText(hdc, g.bottomView.x + 52, g.bottomView.y + 98, RGB(245, 245, 255), "Steuerung tauschen");
        drawText(hdc, g.bottomView.x + 214, g.bottomView.y + 98, RGB(210, 220, 240),
                 g.menu.getControlsSwapped() ? "AN" : "AUS");
        drawText(hdc, g.bottomView.x + 62, g.bottomView.y + 146, RGB(250, 240, 220), "Kartenfortschritt reset");
        drawText(hdc, g.bottomView.x + 132, g.bottomView.y + 194, RGB(245, 245, 255), "Zurueck");
    } else if (state == MENU_MANUAL_LIST) {
        for (int i = 0; i < manualTopicCount(); ++i) {
            COLORREF c = (g.menu.getManualSelection() == i) ? RGB(84, 140, 220) : RGB(56, 72, 104);
            fillRect(hdc, g.bottomView.x + 20, g.bottomView.y + 40 + i * 36, 280, 32, c);
            drawText(hdc, g.bottomView.x + 28, g.bottomView.y + 50 + i * 36, RGB(245, 245, 255), manualTopicName(i));
        }
        COLORREF backCol = (g.menu.getManualSelection() == manualTopicCount()) ? RGB(124, 96, 186) : RGB(70, 58, 102);
        fillRect(hdc, g.bottomView.x + 20, g.bottomView.y + 220, 280, 18, backCol);
        drawText(hdc, g.bottomView.x + 126, g.bottomView.y + 222, RGB(245, 245, 255), "Zurueck");
    } else {
        int topic = g.menu.getManualSelection();
        if (topic >= manualTopicCount()) topic = manualTopicCount() - 1;
        if (topic < 0) topic = 0;
        fillRect(hdc, g.bottomView.x + 12, g.bottomView.y + 12, 296, 30, RGB(56, 72, 104));
        drawText(hdc, g.bottomView.x + 18, g.bottomView.y + 20, RGB(245, 245, 255), manualTopicName(topic));
        fillRect(hdc, g.bottomView.x + 12, g.bottomView.y + 52, 296, 130, RGB(36, 46, 66));
        drawText(hdc, g.bottomView.x + 22, g.bottomView.y + 64, RGB(208, 220, 244), "Oben/unten tippen oder DPad");
        drawText(hdc, g.bottomView.x + 22, g.bottomView.y + 84, RGB(208, 220, 244), "fuer Scroll im Handbuchtext.");
        drawText(hdc, g.bottomView.x + 22, g.bottomView.y + 108, RGB(182, 198, 230), "A/Y oder B: Zur Themenliste");
        char sc[64] = {0};
        int maxScroll = manualMaxScrollForViewport(topic, 12);
        std::snprintf(sc, sizeof(sc), "Scroll: %d / %d", g.menu.getManualScroll(), maxScroll);
        drawText(hdc, g.bottomView.x + 22, g.bottomView.y + 132, RGB(182, 198, 230), sc);
    }
}

void renderFrame(HDC hdc) {
    // Zeichnet ein komplettes Frame abhängig vom App-Zustand.
    RECT full{};
    GetClientRect(g.hwnd, &full);
    fillRect(hdc, full.left, full.top, full.right - full.left, full.bottom - full.top, RGB(16, 20, 32));

    fillRect(hdc, g.renderArea.x, g.renderArea.y, g.renderArea.w, g.renderArea.h, RGB(16, 20, 32));

    if (g.appState == APP_MAIN_MENU) {
        drawTopMenu(hdc);
        drawBottomMenu(hdc);
    } else {
        drawTopGameplay(hdc);
        drawBottomGameplay(hdc);
    }

}

void presentFrame() {
    if (!g.hwnd) return;

    RECT client{};
    GetClientRect(g.hwnd, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;
    if (width <= 0 || height <= 0) return;

    HDC hdc = GetDC(g.hwnd);
    if (!hdc) return;

    if (ensureBackBuffer(hdc, width, height)) {
        renderFrame(g.backDc);
        BitBlt(hdc, 0, 0, width, height, g.backDc, 0, 0, SRCCOPY);
    } else {
        renderFrame(hdc);
    }

    ReleaseDC(g.hwnd, hdc);
}

void releaseBackBuffer() {
    if (g.backDc && g.backBmp) {
        if (g.backOldBmp) {
            SelectObject(g.backDc, g.backOldBmp);
            g.backOldBmp = nullptr;
        }
        DeleteObject(g.backBmp);
        g.backBmp = nullptr;
    }
    if (g.backDc) {
        DeleteDC(g.backDc);
        g.backDc = nullptr;
    }
    g.backW = 0;
    g.backH = 0;
}

bool ensureBackBuffer(HDC referenceDc, int width, int height) {
    if (width <= 0 || height <= 0) return false;
    if (g.backDc && g.backBmp && g.backW == width && g.backH == height) return true;

    releaseBackBuffer();

    g.backDc = CreateCompatibleDC(referenceDc);
    if (!g.backDc) return false;

    g.backBmp = CreateCompatibleBitmap(referenceDc, width, height);
    if (!g.backBmp) {
        releaseBackBuffer();
        return false;
    }

    g.backOldBmp = static_cast<HBITMAP>(SelectObject(g.backDc, g.backBmp));
    g.backW = width;
    g.backH = height;
    return true;
}

void tick() {
    // Simulations-Takt: je nach Zustand Menü- oder Gameplaylogik ausführen.
    if (g.appState == APP_MAIN_MENU) {
        handleMenuLogic();
    } else {
        handleGameplayInput();
        updateGameplay(1.0f / 60.0f);

        if (g.requestMenu) {
            g.requestMenu = false;
            g.appState = APP_MAIN_MENU;
        }
    }

    syncSettingsToDiskIfChanged();

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;

        case WM_KEYDOWN:
            mapVKeyToKeys(static_cast<UINT>(wParam), true);
            return 0;

        case WM_KEYUP:
            mapVKeyToKeys(static_cast<UINT>(wParam), false);
            return 0;

        case WM_LBUTTONDOWN:
            queueTouchFromMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps{};
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE:
            releaseBackBuffer();
            return 0;

        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            if (!mmi) return 0;
            RECT rc{0, 0, g.renderArea.w, g.renderArea.h};
            DWORD style = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_STYLE));
            DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_EXSTYLE));
            AdjustWindowRectEx(&rc, style, FALSE, exStyle);
            mmi->ptMinTrackSize.x = rc.right - rc.left;
            mmi->ptMinTrackSize.y = rc.bottom - rc.top;
            return 0;
        }

        case WM_DESTROY:
            persistRuntimeState();
            syncSettingsToDiskIfChanged();
            releaseMapCache();
            releaseBackBuffer();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd) {
    // Win32-Einstiegspunkt: Initialisierung, Message Loop, 60Hz Taktung.
    timeBeginPeriod(1);

    const char* mapsRootEnv = std::getenv("METROID_MAPS_ROOT");
    const char* saveRootEnv = std::getenv("METROID_SAVE_ROOT");
    g.mapsRoot = mapsRootEnv ? mapsRootEnv : "romfs/maps";

    if (saveRootEnv && saveRootEnv[0] != '\0') {
        g.saveRoot = normalizeSlashes(saveRootEnv);
    } else {
        g.saveRoot = localAppDataDir() + "/MetroidDesktopSimulator/sim_saves";
    }
    ensureSaveDir();

    g.menu.reset();
    g.menu.setHasContinue(false);

    bool loadedDebug = g.savedDebugEnabled;
    bool loadedControls = g.savedControlsSwapped;
    bool loadedFps = g.savedShowFps;
    if (loadSettingsFromDisk(loadedDebug, loadedControls, loadedFps)) {
        g.savedDebugEnabled = loadedDebug;
        g.savedControlsSwapped = loadedControls;
        g.savedShowFps = loadedFps;
    }
    g.menu.setDebugEnabled(g.savedDebugEnabled);
    g.menu.setControlsSwapped(g.savedControlsSwapped);
    g.showFpsEnabled = g.savedShowFps;
    g.controlsSwapped = g.menu.getControlsSwapped();
    saveSettingsToDisk(g.savedDebugEnabled, g.savedControlsSwapped, g.savedShowFps);

    const char* className = "MetroidDesktopSimulatorClass";
    WNDCLASSA wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    if (!RegisterClassA(&wc)) {
        MessageBoxA(nullptr, "RegisterClass fehlgeschlagen.", "Simulator", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT initialRect{0, 0, g.renderArea.w, g.renderArea.h};
    AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    int initialWindowW = initialRect.right - initialRect.left;
    int initialWindowH = initialRect.bottom - initialRect.top;

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "Metroid Desktop Simulator (MainMenu + Gameplay)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        initialWindowW,
        initialWindowH,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!hwnd) {
        MessageBoxA(nullptr, "Fenster konnte nicht erstellt werden.", "Simulator", MB_OK | MB_ICONERROR);
        return 1;
    }

    g.hwnd = hwnd;
    g.fpsTimerMs = nowMs();

    std::string worldPath = g.mapsRoot + "/world.json";
    if (!g.world.loadWorldJson(worldPath.c_str())) {
        MessageBoxA(hwnd, "Konnte world.json nicht laden. Fuehre zuerst den Map-Export aus.", "Simulator", MB_OK | MB_ICONERROR);
        return 1;
    }
    g.world.buildSpatialMap(g.mapsRoot.c_str(), 16, 400, 240);

    ShowWindow(hwnd, showCmd);
    UpdateWindow(hwnd);
    presentFrame();

    LARGE_INTEGER freq{};
    LARGE_INTEGER prev{};
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    const double targetDt = 1.0 / 60.0;
    const int maxCatchupSteps = 3;
    double accumulator = 0.0;
    bool running = true;

    while (running) {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        double dt = static_cast<double>(now.QuadPart - prev.QuadPart) / static_cast<double>(freq.QuadPart);
        prev = now;
        if (dt > 0.25) dt = 0.25;
        accumulator += dt;

        bool stepped = false;
        int stepCount = 0;
        while (accumulator >= targetDt && stepCount < maxCatchupSteps) {
            tick();
            accumulator -= targetDt;
            stepped = true;
            ++stepCount;
        }

        if (stepCount == maxCatchupSteps && accumulator >= targetDt) {
            accumulator = 0.0;
        }

        if (stepped) {
            presentFrame();
        } else {
            double msToNextStep = (targetDt - accumulator) * 1000.0;
            if (msToNextStep > 1.5) {
                Sleep(static_cast<DWORD>(msToNextStep - 0.5));
            } else {
                Sleep(0);
            }
        }
    }

    timeEndPeriod(1);
    return 0;
}
