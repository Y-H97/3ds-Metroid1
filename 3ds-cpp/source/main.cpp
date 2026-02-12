#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <string>

#include "core/game_core.h"
#include "core/world_map.h"

constexpr u32 CLEAR_COLOR = C2D_Color32(16, 20, 32, 255);
constexpr u32 TILE_COLOR  = C2D_Color32(90, 180, 220, 255);
constexpr u32 BG_COLOR    = C2D_Color32(10, 14, 20, 255);

enum BottomTab {
    TAB_MAP = 0,
    TAB_INVENTORY = 1,
    TAB_SETTINGS = 2,
    TAB_DEBUG = 3,
};

enum AppState {
    APP_MAIN_MENU = 0,
    APP_GAME = 1,
};

enum MainMenuState {
    MENU_MAIN = 0,
    MENU_OPTIONS = 1,
};

struct TextRenderer {
    C2D_TextBuf buf = nullptr;

    bool init(size_t capacity = 4096) {
        buf = C2D_TextBufNew(capacity);
        return buf != nullptr;
    }

    void beginFrame() {
        if (buf) C2D_TextBufClear(buf);
    }

    void draw(float x, float y, float scale, u32 color, const char* fmt, ...) {
        if (!buf || !fmt) return;
        char line[256];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(line, sizeof(line), fmt, args);
        va_end(args);

        C2D_Text text;
        C2D_TextParse(&text, buf, line);
        C2D_TextOptimize(&text);
        C2D_DrawText(&text, C2D_WithColor, x, y, 0.3f, scale, scale, color);
    }

    void shutdown() {
        if (buf) {
            C2D_TextBufDelete(buf);
            buf = nullptr;
        }
    }
};

struct TileRenderer {
    C2D_SpriteSheet sheet = nullptr;
    int tileSize = 16;

    bool init(const char* t3xPath, int size) {
        tileSize = size;
        sheet = C2D_SpriteSheetLoad(t3xPath);
        return sheet != nullptr;
    }

    void drawSlope(float x, float y, int tileIndex) const {
        u32 col = C2D_Color32(160, 160, 235, 255);
        if (tileIndex == 32 || tileIndex == 33) {
            col = C2D_Color32(118, 118, 190, 255);
        }
        const float s = static_cast<float>(tileSize);
        C2D_DrawRectSolid(x, y, 0.0f, s, s, C2D_Color32(28, 34, 50, 255));
        if (tileIndex == 30) {
            C2D_DrawTriangle(x, y + s, col, x + s, y + s, col, x + s, y, col, 0.05f);
        } else if (tileIndex == 31) {
            C2D_DrawTriangle(x, y, col, x, y + s, col, x + s, y + s, col, 0.05f);
        } else if (tileIndex == 32) {
            C2D_DrawTriangle(x, y, col, x + s, y, col, x + s, y + s, col, 0.05f);
        } else if (tileIndex == 33) {
            C2D_DrawTriangle(x, y, col, x + s, y, col, x, y + s, col, 0.05f);
        }
    }

    void drawTile(float x, float y, int tileIndex) const {
        if (tileIndex == 30 || tileIndex == 31 || tileIndex == 32 || tileIndex == 33) {
            drawSlope(x, y, tileIndex);
            return;
        }
        if (sheet) {
            C2D_Image img = C2D_SpriteSheetGetImage(sheet, tileIndex % C2D_SpriteSheetCount(sheet));
            C2D_DrawImageAt(img, x, y, 0.0f, nullptr, 1.0f, 1.0f);
        } else {
            u32 col = TILE_COLOR;
            if (tileIndex == 1) col = C2D_Color32(128, 128, 204, 255);
            else if (tileIndex == 2) col = C2D_Color32(230, 51, 51, 255);
            else if (tileIndex >= 30) {
                if (tileIndex == 30 || tileIndex == 31) col = C2D_Color32(153, 153, 230, 255);
                else col = C2D_Color32(102, 102, 179, 255);
            } else if (tileIndex >= 4) {
                switch (tileIndex) {
                    case 4: col = C2D_Color32(51, 51, 51, 255); break;
                    case 5: col = C2D_Color32(26, 26, 77, 255); break;
                    case 6: col = C2D_Color32(26, 51, 26, 255); break;
                    case 7: col = C2D_Color32(51, 26, 51, 255); break;
                    case 8: col = C2D_Color32(38, 38, 38, 255); break;
                    case 9: col = C2D_Color32(20, 20, 20, 255); break;
                    case 10: col = C2D_Color32(46, 56, 64, 255); break;
                    case 11: col = C2D_Color32(13, 13, 51, 255); break;
                    case 12: col = C2D_Color32(0, 51, 64, 255); break;
                    case 13: col = C2D_Color32(13, 38, 13, 255); break;
                    case 14: col = C2D_Color32(51, 51, 26, 255); break;
                    case 15: col = C2D_Color32(77, 26, 26, 255); break;
                    case 16: col = C2D_Color32(77, 51, 26, 255); break;
                    case 17: col = C2D_Color32(51, 38, 20, 255); break;
                    case 18: col = C2D_Color32(64, 26, 13, 255); break;
                    case 19: col = C2D_Color32(0, 64, 64, 255); break;
                    case 20: col = C2D_Color32(38, 46, 31, 255); break;
                    case 21: col = C2D_Color32(89, 38, 26, 255); break;
                    default: col = TILE_COLOR; break;
                }
            }
            C2D_DrawRectSolid(x, y, 0.0f, static_cast<float>(tileSize), static_cast<float>(tileSize), col);
        }
    }

    void shutdown() {
        if (sheet) {
            C2D_SpriteSheetFree(sheet);
            sheet = nullptr;
        }
    }
};

static void renderMap(C3D_RenderTarget* target, float originX, float originY, float tileSize, const TileRenderer& renderer, const TileMap& map) {
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

static void drawPlayer(float x, float y, float w, float h) {
    C2D_DrawRectSolid(x, y, 0.1f, w, h, C2D_Color32(250, 210, 60, 255));
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void computeCamera(const Player& p, const TileMap& map, float tileSize, float viewW, float viewH, float& outCamX, float& outCamY) {
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

static void renderBottomUI(
    TextRenderer& text,
    const WorldMap& world,
    const GameCore& core,
    int gridX,
    int gridY,
    int currentGridX,
    int currentGridY,
    int mode,
    bool debugEnabled,
    bool showFpsEnabled,
    int settingsSelection
) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(20, 20, 30, 255));

    C2D_DrawRectSolid(6, 8, 0.0f, 308, 208, C2D_Color32(18, 24, 34, 255));

    if (mode == TAB_MAP) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Karte");
        text.draw(214.0f, 12.0f, 0.36f, C2D_Color32(170, 180, 205, 255), "GX:%d GY:%d", currentGridX, currentGridY);

        int minX = 0;
        int minY = 0;
        int maxX = 0;
        int maxY = 0;
        if (world.getSpatialBounds(minX, minY, maxX, maxY)) {

            int cols = (maxX - minX) + 1;
            int rows = (maxY - minY) + 1;

            int panelX = 12;
            int panelY = 28;
            int panelW = 296;
            int panelH = 180;
            int cellW = panelW / (cols > 0 ? cols : 1);
            int cellH = panelH / (rows > 0 ? rows : 1);
            int cellSize = cellW < cellH ? cellW : cellH;
            if (cellSize < 6) cellSize = 6;
            if (cellSize > 20) cellSize = 20;

            int drawW = cols * cellSize;
            int drawH = rows * cellSize;
            int originX = panelX + (panelW - drawW) / 2;
            int originY = panelY + (panelH - drawH) / 2;

            C2D_DrawRectSolid(originX - 2, originY - 2, 0.0f, drawW + 4, drawH + 4, C2D_Color32(36, 46, 62, 255));

            for (int gy = minY; gy <= maxY; ++gy) {
                for (int gx = minX; gx <= maxX; ++gx) {
                    const SpatialCell* sc = world.getCell(gx, gy);
                    if (!sc) continue;

                    int cx = originX + (gx - minX) * cellSize;
                    int cy = originY + (gy - minY) * cellSize;
                    bool isCurrent = (gx == currentGridX && gy == currentGridY);
                    bool isOrigin = (gx == sc->originX && gy == sc->originY);
                    u32 color = isCurrent ? C2D_Color32(100, 220, 130, 255) : (isOrigin ? C2D_Color32(102, 150, 210, 255) : C2D_Color32(76, 112, 164, 255));
                    C2D_DrawRectSolid(cx, cy, 0.0f, cellSize - 1, cellSize - 1, color);
                    if (isCurrent) {
                        C2D_DrawRectSolid(cx + 2, cy + 2, 0.1f, cellSize - 5, cellSize - 5, C2D_Color32(190, 255, 210, 255));
                    } else if (isOrigin && cellSize >= 8) {
                        C2D_DrawRectSolid(cx + cellSize / 3, cy + cellSize / 3, 0.1f, 2, 2, C2D_Color32(210, 230, 255, 255));
                    }
                }
            }
        }
    } else if (mode == TAB_INVENTORY) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Inventar");
        C2D_DrawRectSolid(16, 44, 0.0f, 288, 144, C2D_Color32(28, 34, 46, 255));
        text.draw(44.0f, 108.0f, 0.46f, C2D_Color32(170, 180, 205, 255), "(Aktuell leer)");
    } else if (mode == TAB_SETTINGS) {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Einstellungen");

        u32 s0 = settingsSelection == 0 ? C2D_Color32(76, 112, 182, 255) : C2D_Color32(46, 62, 92, 255);
        u32 s1 = settingsSelection == 1 ? C2D_Color32(182, 84, 84, 255) : C2D_Color32(92, 46, 46, 255);

        C2D_DrawRectSolid(20, 54, 0.0f, 280, 42, s0);
        C2D_DrawRectSolid(20, 104, 0.0f, 280, 42, s1);
        C2D_DrawRectSolid(20, 160, 0.0f, 280, 30, C2D_Color32(30, 38, 52, 255));

        text.draw(28.0f, 67.0f, 0.40f, C2D_Color32(236, 236, 245, 255), "FPS Anzeige");
        text.draw(248.0f, 67.0f, 0.44f, showFpsEnabled ? C2D_Color32(130, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), showFpsEnabled ? "AN" : "AUS");
        text.draw(28.0f, 117.0f, 0.44f, C2D_Color32(245, 236, 236, 255), "Spiel verlassen");
        text.draw(28.0f, 168.0f, 0.34f, C2D_Color32(170, 180, 205, 255), "Steuerung: Unten/Oben + A");
    } else {
        text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Debug");
        C2D_DrawRectSolid(16, 38, 0.0f, 288, 166, C2D_Color32(24, 30, 42, 255));

        if (!debugEnabled) {
            text.draw(26.0f, 110.0f, 0.42f, C2D_Color32(180, 190, 210, 255), "Debug ist im Hauptmenue AUS");
        } else {
            const Player& p = core.getPlayer();
            const TileMap& map = core.getMap();
            text.draw(24.0f, 48.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Grid: %d, %d", gridX, gridY);
            text.draw(24.0f, 68.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Player X/Y: %.1f / %.1f", p.x, p.y);
            text.draw(24.0f, 88.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Vx/Vy: %.1f / %.1f", p.vx, p.vy);
            text.draw(24.0f, 108.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Grounded: %s", p.grounded ? "ja" : "nein");
            text.draw(24.0f, 128.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Map: %d x %d", map.width, map.height);
            text.draw(24.0f, 148.0f, 0.36f, C2D_Color32(190, 210, 235, 255), "Transitions: %d", static_cast<int>(map.getTransitions().size()));
        }
    }

    u32 tabBg = C2D_Color32(30, 30, 45, 255);
    C2D_DrawRectSolid(0, 220, 0.0f, 320, 20, tabBg);

    const char* tabNames[4] = {"Map", "Inv", "Set", "Dbg"};
    for (int i = 0; i < 4; ++i) {
        bool selected = (i == mode);
        u32 col = selected ? C2D_Color32(80, 140, 220, 255) : C2D_Color32(50, 50, 70, 255);
        C2D_DrawRectSolid(i * 80, 220, 0.0f, 80, 20, col);
        text.draw(i * 80.0f + 28.0f, 224.0f, 0.34f, C2D_Color32(235, 235, 245, 255), "%s", tabNames[i]);
    }
}

static void drawMainMenuTop(TextRenderer& text) {
    C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(16, 20, 32, 255));
    C2D_DrawRectSolid(36, 52, 0.0f, 328, 136, C2D_Color32(32, 40, 60, 255));
    text.draw(88.0f, 84.0f, 0.75f, C2D_Color32(245, 245, 255, 255), "Metroidvania 3DS");
    text.draw(120.0f, 124.0f, 0.42f, C2D_Color32(180, 190, 215, 255), "C++ Core Build");
}

static void drawMainMenuBottomMain(TextRenderer& text, int selection) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
    text.draw(94.0f, 14.0f, 0.50f, C2D_Color32(235, 235, 245, 255), "Hauptmenue");

    u32 c0 = selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c1 = selection == 1 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c2 = selection == 2 ? C2D_Color32(176, 84, 84, 255) : C2D_Color32(96, 50, 50, 255);

    C2D_DrawRectSolid(60, 48, 0.0f, 200, 45, c0);
    C2D_DrawRectSolid(60, 104, 0.0f, 200, 45, c1);
    C2D_DrawRectSolid(60, 160, 0.0f, 200, 45, c2);

    text.draw(106.0f, 62.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Neues Spiel");
    text.draw(98.0f, 118.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Einstellungen");
    text.draw(94.0f, 174.0f, 0.44f, C2D_Color32(250, 236, 236, 255), "Spiel beenden");
}

static void drawMainMenuBottomOptions(TextRenderer& text, int selection, bool debugEnabled) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
    text.draw(84.0f, 14.0f, 0.50f, C2D_Color32(235, 235, 245, 255), "Einstellungen");

    u32 c0 = selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c1 = selection == 1 ? C2D_Color32(124, 96, 186, 255) : C2D_Color32(70, 58, 102, 255);

    C2D_DrawRectSolid(40, 60, 0.0f, 240, 50, c0);
    C2D_DrawRectSolid(40, 124, 0.0f, 240, 50, c1);

    text.draw(52.0f, 76.0f, 0.40f, C2D_Color32(245, 245, 255, 255), "Ingame Debug Info");
    text.draw(228.0f, 76.0f, 0.44f, debugEnabled ? C2D_Color32(120, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), debugEnabled ? "AN" : "AUS");
    text.draw(132.0f, 140.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Zurueck");
}

static std::string pickFirstLevelFromWorld(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return {};
    std::string s; char buf[512];
    while (size_t n = fread(buf, 1, sizeof(buf), f)) s.append(buf, n);
    fclose(f);
    // look for "level":"NAME"
    size_t pos = s.find("\"level\"");
    if (pos == std::string::npos) return {};
    pos = s.find('"', pos + 7);
    if (pos == std::string::npos) return {};
    size_t end = s.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return s.substr(pos + 1, end - pos - 1);
}

int main() {
    gfxInitDefault();
    romfsInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    GameCore core;
    WorldMap world;
    // Choose first level from exported world.json; fallback to demo.txt
    std::string levelName = pickFirstLevelFromWorld("romfs:/maps/world.json");
    bool mapOk = false;
    int gridX = 0;
    int gridY = 0;
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

    TileRenderer renderer;
    TextRenderer text;
    // Tries to load romfs:/gfx/tiles.t3x. If missing, falls back to solid color.
    renderer.init("romfs:/gfx/tiles.t3x", 16);
    text.init();

    if (!core.setPlayerStartToFirstEmpty(16.0f)) {
        core.setPlayerStart(40.0f, 40.0f);
    }

    struct Checkpoint {
        bool valid = false;
        std::string level;
        int gridX = 0;
        int gridY = 0;
        float x = 0.0f;
        float y = 0.0f;
    } checkpoint;

    checkpoint.valid = true;
    checkpoint.level = levelName;
    checkpoint.gridX = gridX;
    checkpoint.gridY = gridY;
    checkpoint.x = core.getPlayer().x;
    checkpoint.y = core.getPlayer().y;

    int bottomMode = TAB_MAP;
    bool debugInfoEnabled = true;
    bool showFpsEnabled = false;
    int settingsSelection = 0;
    bool inTransition = false;
    bool requestExit = false;

    AppState appState = APP_MAIN_MENU;
    MainMenuState mainMenuState = MENU_MAIN;
    int mainMenuSelection = 0;
    int optionsSelection = 0;

    u64 fpsTimerMs = osGetTime();
    int fpsFrameCounter = 0;
    int fpsValue = 0;
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START) break;

        if (appState == APP_MAIN_MENU) {
            if (kDown & KEY_UP) {
                if (mainMenuState == MENU_MAIN) mainMenuSelection = (mainMenuSelection + 2) % 3;
                else optionsSelection = (optionsSelection + 1) % 2;
            }
            if (kDown & KEY_DOWN) {
                if (mainMenuState == MENU_MAIN) mainMenuSelection = (mainMenuSelection + 1) % 3;
                else optionsSelection = (optionsSelection + 1) % 2;
            }

            if (kDown & KEY_B) {
                if (mainMenuState == MENU_OPTIONS) mainMenuState = MENU_MAIN;
            }

            if (kDown & KEY_A) {
                if (mainMenuState == MENU_MAIN) {
                    if (mainMenuSelection == 0) {
                        appState = APP_GAME;
                    } else if (mainMenuSelection == 1) {
                        mainMenuState = MENU_OPTIONS;
                        optionsSelection = 0;
                    } else {
                        requestExit = true;
                    }
                } else {
                    if (optionsSelection == 0) {
                        debugInfoEnabled = !debugInfoEnabled;
                    } else {
                        mainMenuState = MENU_MAIN;
                    }
                }
            }

            if (kDown & KEY_TOUCH) {
                touchPosition tp;
                hidTouchRead(&tp);
                if (mainMenuState == MENU_MAIN) {
                    if (tp.py >= 48 && tp.py <= 93) {
                        mainMenuSelection = 0;
                        appState = APP_GAME;
                    } else if (tp.py >= 104 && tp.py <= 149) {
                        mainMenuSelection = 1;
                        mainMenuState = MENU_OPTIONS;
                        optionsSelection = 0;
                    } else if (tp.py >= 160 && tp.py <= 205) {
                        mainMenuSelection = 2;
                        requestExit = true;
                    }
                } else {
                    if (tp.py >= 60 && tp.py <= 110) {
                        optionsSelection = 0;
                        debugInfoEnabled = !debugInfoEnabled;
                    } else if (tp.py >= 124 && tp.py <= 174) {
                        optionsSelection = 1;
                        mainMenuState = MENU_MAIN;
                    }
                }
            }
        } else {
            if (kDown & KEY_SELECT) {
                appState = APP_MAIN_MENU;
                mainMenuState = MENU_MAIN;
                mainMenuSelection = 0;
            }

            if (kDown & KEY_X) bottomMode = TAB_MAP;
            if (kDown & KEY_Y) bottomMode = TAB_INVENTORY;
            if (kDown & KEY_L) bottomMode = TAB_SETTINGS;
            if (kDown & KEY_R) bottomMode = TAB_DEBUG;

            if (kDown & KEY_TOUCH) {
                touchPosition tp;
                hidTouchRead(&tp);
                if (tp.py >= 220) {
                    bottomMode = tp.px / 80;
                    if (bottomMode < 0) bottomMode = 0;
                    if (bottomMode > 3) bottomMode = 3;
                } else if (bottomMode == TAB_SETTINGS) {
                    if (tp.py >= 54 && tp.py <= 96) {
                        settingsSelection = 0;
                        showFpsEnabled = !showFpsEnabled;
                    } else if (tp.py >= 104 && tp.py <= 146) {
                        settingsSelection = 1;
                        requestExit = true;
                    }
                }
            }

            if (bottomMode == TAB_SETTINGS) {
                if (kDown & KEY_UP) settingsSelection = (settingsSelection + 1) % 2;
                if (kDown & KEY_DOWN) settingsSelection = (settingsSelection + 1) % 2;
                if (kDown & KEY_A) {
                    if (settingsSelection == 0) showFpsEnabled = !showFpsEnabled;
                    else requestExit = true;
                }
            }
        }

        if (requestExit) break;

        const Player& currentPlayer = core.getPlayer();
        int currentGridX = gridX + static_cast<int>(std::floor(currentPlayer.x / 400.0f));
        int currentGridY = gridY + static_cast<int>(std::floor(currentPlayer.y / 240.0f));

        const float dt = 1.0f / 60.0f;
        if (appState == APP_GAME) {
            InputState input;
            input.left = (kHeld & KEY_LEFT) != 0;
            input.right = (kHeld & KEY_RIGHT) != 0;
            input.jump = (kHeld & KEY_A) != 0;
            input.jumpPressed = (kDown & KEY_A) != 0;
            core.update(input, dt);

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

                            if (world.isCheckpoint(nextGX, nextGY)) {
                                checkpoint.valid = true;
                                checkpoint.level = nextCell->level;
                                checkpoint.gridX = nextCell->originX;
                                checkpoint.gridY = nextCell->originY;
                                checkpoint.x = localTargetX;
                                checkpoint.y = localTargetY;
                                core.setPlayerStart(localTargetX, localTargetY);
                            } else {
                                core.setPlayerPosition(localTargetX, localTargetY);
                            }
                        }
                    }
                }
            }

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

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        text.beginFrame();

        if (appState == APP_GAME) {
            float camX = 0.0f;
            float camY = 0.0f;
            computeCamera(core.getPlayer(), core.getMap(), 16.0f, 400.0f, 240.0f, camX, camY);

            C2D_TargetClear(top, CLEAR_COLOR);
            renderMap(top, -camX, -camY, 16.0f, renderer, core.getMap());
            const Player& p = core.getPlayer();
            drawPlayer(p.x - camX, p.y - camY, p.w, p.h);
            for (const auto& tr : core.getMap().getTransitions()) {
                C2D_DrawRectSolid(tr.x - camX, tr.y - camY, 0.2f, tr.w, tr.h, C2D_Color32(0, 220, 0, 255));
            }
            if (showFpsEnabled) {
                text.draw(6.0f, 6.0f, 0.36f, C2D_Color32(120, 255, 140, 255), "FPS: %d", fpsValue);
            }

            C2D_TargetClear(bottom, BG_COLOR);
            C2D_SceneBegin(bottom);
            renderBottomUI(text, world, core, gridX, gridY, currentGridX, currentGridY, bottomMode, debugInfoEnabled, showFpsEnabled, settingsSelection);
        } else {
            C2D_TargetClear(top, CLEAR_COLOR);
            C2D_SceneBegin(top);
            drawMainMenuTop(text);

            C2D_TargetClear(bottom, BG_COLOR);
            C2D_SceneBegin(bottom);
            if (mainMenuState == MENU_MAIN) drawMainMenuBottomMain(text, mainMenuSelection);
            else drawMainMenuBottomOptions(text, optionsSelection, debugInfoEnabled);
        }

        C3D_FrameEnd(0);
    }

    renderer.shutdown();
    text.shutdown();
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
