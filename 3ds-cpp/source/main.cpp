#include <3ds.h>
#include <citro2d.h>

#include "core/app_settings.h"
#include "gameplay/gameplay_scene.h"
#include "menu/controllers/main_menu_controller.h"
#include "ui/text_renderer.h"

// Hauptmodus der Anwendung:
// Entweder befindet sich der Spieler im Hauptmenü oder im eigentlichen Spiel.
enum AppState {
    APP_MAIN_MENU = 0,
    APP_GAME = 1,
};

int main() {
    // 3DS-System, Dateisystem (romfs) und 2D-Renderer starten.
    gfxInitDefault();
    romfsInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    TextRenderer text;
    text.init();

    MainMenuController menu;
    menu.reset();

    GameplayScene gameplay;
    if (!gameplay.init()) {
        text.shutdown();
        C2D_Fini();
        C3D_Fini();
        romfsExit();
        gfxExit();
        return 1;
    }

    AppSettingsData settings{};
    AppSettings::load(settings);
    menu.setDebugEnabled(settings.debugEnabled);
    menu.setControlsSwapped(settings.controlsSwapped);
    gameplay.setShowFpsEnabled(settings.showFps);

    AppSettingsData savedSettings = settings;

    // Laufzeit-Zustand: Wir wechseln während des Spiels zwischen Menü und Gameplay.
    AppState appState = APP_MAIN_MENU;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) break;

        if (appState == APP_MAIN_MENU) {
            // Menü verarbeitet Eingaben und liefert Aktionen zurück (Starten, Laden, Beenden ...).
            int selectedSlot = menu.getSelectedSaveSlot();
            menu.setHasContinue(gameplay.hasPersistentSave(selectedSlot));
            menu.handleKeys(kDown);
            if (kDown & KEY_TOUCH) {
                touchPosition tp;
                hidTouchRead(&tp);
                menu.handleTouch(tp);
            }

            MainMenuAction action = menu.consumeAction();
            if (action.exitGame) break;
            if (action.startGame) {
                if (gameplay.startNewGame(menu.getSelectedSaveSlot())) {
                    appState = APP_GAME;
                    gameplay.clearReturnToMenu();
                }
            }
            if (action.loadCheckpoint) {
                if (gameplay.loadFromCheckpoint(menu.getSelectedSaveSlot())) {
                    appState = APP_GAME;
                    gameplay.clearReturnToMenu();
                }
            }
            if (action.resetMapProgress) {
                gameplay.resetVisitedProgress(menu.getSelectedSaveSlot());
            }
        } else {
            // Gameplay-Phase: Eingaben anwenden, Spielwelt fortschreiben, Sonderfälle prüfen.
            gameplay.setControlsSwapped(menu.getControlsSwapped());
            gameplay.handleInput(kDown, kHeld);
            gameplay.update(1.0f / 60.0f);

            if (gameplay.shouldExitGame()) break;
            if (gameplay.shouldReturnToMenu()) {
                appState = APP_MAIN_MENU;
                gameplay.clearReturnToMenu();
            }
        }

        // Einstellungen werden bei Änderungen auf SD gespeichert,
        // damit sie beim nächsten Start wieder verfügbar sind.
        AppSettingsData currentSettings{};
        currentSettings.debugEnabled = menu.getDebugEnabled();
        currentSettings.controlsSwapped = menu.getControlsSwapped();
        currentSettings.showFps = gameplay.getShowFpsEnabled();
        if (currentSettings.debugEnabled != savedSettings.debugEnabled ||
            currentSettings.controlsSwapped != savedSettings.controlsSwapped ||
            currentSettings.showFps != savedSettings.showFps) {
            if (AppSettings::save(currentSettings)) {
                savedSettings = currentSettings;
            }
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        text.beginFrame();

        if (appState == APP_MAIN_MENU) {
            // Darstellung des Menüs auf Top- und Bottom-Screen.
            C2D_TargetClear(top, C2D_Color32(16, 20, 32, 255));
            C2D_SceneBegin(top);
            menu.renderTop(text);

            C2D_TargetClear(bottom, C2D_Color32(10, 14, 20, 255));
            C2D_SceneBegin(bottom);
            menu.renderBottom(text);
        } else {
            // Darstellung des Spiels: oben Welt, unten Tabs/UI.
            gameplay.renderTop(top, text, menu.getDebugEnabled());
            gameplay.renderBottom(bottom, text, menu.getDebugEnabled());
        }

        C3D_FrameEnd(0);
    }

    gameplay.shutdown();
    text.shutdown();
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
