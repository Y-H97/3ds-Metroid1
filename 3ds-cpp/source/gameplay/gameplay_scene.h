#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <string>
#include <unordered_set>

#include "../core/game_core.h"
#include "../core/world_map.h"
#include "render/tile_renderer.h"

class TextRenderer;

// Zentrale Gameplay-Orchestrierung.
// Verantwortlich für:
// - Laden/Wechseln von Kartenräumen
// - Player-Input -> GameCore
// - Persistenz (Save-Slots + Fog-of-War/visited)
// - Render-Aufbereitung für Top/Bottom Screen
class GameplayScene {
public:
    static constexpr int SAVE_SLOT_COUNT = 3;

    // Initialisiert Renderer, Welt und Startzustände.
    bool init();
    // Gibt Ressourcen frei und persistiert den letzten Zustand.
    void shutdown();
    // Eingabeschema zwischen CirclePad und DPad tauschen.
    void setControlsSwapped(bool swapped);
    // FPS-Overlay ein-/ausschalten.
    void setShowFpsEnabled(bool enabled);
    bool getShowFpsEnabled() const;
    // Aktiven Save-Slot setzen/lesen.
    void setActiveSaveSlot(int slot);
    int getActiveSaveSlot() const;

    // Roh-Eingaben entgegennehmen und in Szenen-Zustand überführen.
    void handleInput(u32 kDown, u32 kHeld);
    // Spiellogik für ein Frame fortschreiben.
    void update(float dt);
    // Neues Spiel für Slot starten.
    bool startNewGame(int slot);
    // Letzten Checkpoint eines Slots laden.
    bool loadFromCheckpoint(int slot);
    // Prüft, ob für Slot ein Spielstand existiert.
    bool hasPersistentSave(int slot);
    // Setzt nur den Kartenfortschritt eines Slots zurück.
    bool resetVisitedProgress(int slot);

    // Frage oder verändere den Zustand spezieller Upgrades/Items
    // (aktuell nur Doppelsprung). Diese Methoden fassen Operationen zusammen,
    // welche von externen Modulen wie item-spezifischen Klassen aufgerufen
    // werden können.
    void grantDoubleJump();
    bool playerHasDoubleJump() const;

    void renderTop(C3D_RenderTarget* top, TextRenderer& text, bool debugInfoEnabled);
    void renderBottom(C3D_RenderTarget* bottom, TextRenderer& text, bool debugInfoEnabled);

    bool shouldExitGame() const;
    bool shouldReturnToMenu() const;
    void clearReturnToMenu();

private:
    // Laufzeit-Checkpoint für Respawn + persistentes Speichern.
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
        // Gesammelte und aktivierte Items (Bitmasken).
        uint32_t collectedItems = 0;
        uint32_t activeItems = 0;
    };

    enum BottomTab {
        TAB_MAP = 0,
        TAB_INVENTORY = 1,
        TAB_SETTINGS = 2,
        TAB_DEBUG = 3,
    };

    // Interne Hilfsfunktionen für Welt-Start und Persistenz pro Save-Slot.
    bool loadInitialMap();
    bool loadPersistentSaveFromDisk(int slot, PersistentSave* outSave = nullptr);
    bool writePersistentSaveToDisk(const Checkpoint& cp, int slot);
    bool loadVisitedFromDisk(int slot);
    bool writeVisitedToDisk(int slot) const;

    GameCore core;
    WorldMap world;
    TileRenderer renderer;
    Checkpoint checkpoint;
    PersistentSave persistentSave;

    int gridX = 0;
    int gridY = 0;
    int currentGridX = 0;
    int currentGridY = 0;

    // Items, die sich in der aktuell geladenen Karte befinden.
    struct MapItem {
        std::string type;
        float x = 0.0f;
        float y = 0.0f;
        bool collected = false;
    };
    std::vector<MapItem> mapItems;

    // Runtime-Zustand für Items
    uint32_t collectedItems = 0; // welche Items wurden eingesammelt
    uint32_t activeItems = 0;    // welche Items sind aktuell aktiviert (ON/OFF)
    int inventorySelection = 0;  // Index des ausgewählten Eintrags im Inventar
    std::string pickupMessage;   // Text, der angezeigt werden soll sobald ein Item eingesammelt wird
    float pickupMessageTimer = 0.0f; // verbleibende Zeit für die Anzeige

    int bottomMode = TAB_MAP;
    int settingsSelection = 0;
    bool showFpsEnabled = false;
    bool inTransition = false;
    bool requestExit = false;
    bool requestMenu = false;

    u64 fpsTimerMs = 0;
    int fpsFrameCounter = 0;
    int fpsValue = 0;

    std::string currentLevelName;
    float lastCamX = 0.0f;
    float lastCamY = 0.0f;
    int playerTileX = 0;
    int playerTileY = 0;
    int playerTileId = 0;
    int debugScrollPx = 0;
    int debugMaxScrollPx = 0;
    bool controlsSwapped = false;
    bool moveLeftHeld = false;
    bool moveRightHeld = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    // Aktiver Save-Slot (1..SAVE_SLOT_COUNT), bestimmt Save/Visited-Dateien.
    int activeSaveSlot = 1;
    std::unordered_set<std::string> visitedCells;
};
