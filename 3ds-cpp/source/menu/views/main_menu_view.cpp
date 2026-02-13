#include "main_menu_view.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

static void drawPanel(float x, float y, float w, float h, u32 bg) {
    C2D_DrawRectSolid(x, y, 0.0f, w, h, bg);
}

void drawMainMenuTopView(TextRenderer& text, int state, int activeSelection, int selectedSaveSlot, bool hasContinue, bool debugEnabled, bool controlsSwapped) {
    C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(16, 20, 32, 255));
    drawPanel(16, 14, 368, 34, C2D_Color32(30, 40, 62, 255));
    text.draw(26.0f, 24.0f, 0.46f, C2D_Color32(244, 248, 255, 255), "Metroidvania 3DS");
    text.draw(250.0f, 24.0f, 0.34f, C2D_Color32(180, 194, 224, 255), "C++ Core Build");

    drawPanel(16, 58, 140, 166, C2D_Color32(26, 34, 52, 255));
    drawPanel(164, 58, 220, 166, C2D_Color32(32, 42, 64, 255));

    const char* categories[3] = {"Spielen", "Optionen", "Beenden"};
    for (int i = 0; i < 3; ++i) {
        bool selected = (i == state);
        u32 rowCol = selected ? C2D_Color32(74, 118, 194, 255) : C2D_Color32(42, 56, 84, 255);
        drawPanel(24, 70 + i * 38, 124, 30, rowCol);
        text.draw(34.0f, 79.0f + i * 38, 0.38f, C2D_Color32(238, 242, 250, 255), "%s", categories[i]);
    }

    if (state == 2) {
        text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "Einstellungs-Hilfe");
        if (activeSelection == 0) {
                text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "Ingame Debug Info");
                text.draw(176.0f, 118.0f, 0.33f, C2D_Color32(182, 198, 230, 255), "Zeigt technische Werte ingame an.");
                text.draw(176.0f, 140.0f, 0.33f, debugEnabled ? C2D_Color32(136, 240, 170, 255) : C2D_Color32(220, 170, 170, 255), "Aktuell: %s", debugEnabled ? "AN" : "AUS");
        } else if (activeSelection == 1) {
                text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "Steuerung tauschen");
                text.draw(176.0f, 118.0f, 0.33f, C2D_Color32(182, 198, 230, 255), "Wechselt Top/Bottom Eingabe-Schema.");
                text.draw(176.0f, 140.0f, 0.33f, controlsSwapped ? C2D_Color32(136, 240, 170, 255) : C2D_Color32(220, 170, 170, 255), "Aktuell: %s", controlsSwapped ? "TAUSCH" : "STANDARD");
        } else if (activeSelection == 2) {
                text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "Kartenfortschritt reset");
                text.draw(176.0f, 118.0f, 0.33f, C2D_Color32(182, 198, 230, 255), "Loescht den Fog-of-War des aktiven Slots.");
                text.draw(176.0f, 140.0f, 0.33f, C2D_Color32(250, 200, 146, 255), "Aktion wirkt sofort nach Bestaetigen.");
        } else {
                text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "Zurueck");
                text.draw(176.0f, 118.0f, 0.33f, C2D_Color32(182, 198, 230, 255), "Verlaesst die Optionen und geht ins");
                text.draw(176.0f, 140.0f, 0.33f, C2D_Color32(182, 198, 230, 255), "Hauptmenue zurueck.");
        }
    } else {
        text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "Status");
        text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(190, 206, 236, 255), "Aktiver Slot: %d", selectedSaveSlot);
        text.draw(176.0f, 120.0f, 0.34f, hasContinue ? C2D_Color32(136, 240, 170, 255) : C2D_Color32(220, 170, 170, 255), hasContinue ? "Fortsetzen verfuegbar" : "Kein Save im Slot");
    }

    if (state == 0) {
        const char* labels[3] = {"Kategorie Spielen", "Kategorie Optionen", "Spiel beenden"};
        text.draw(176.0f, 150.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "%s", labels[activeSelection]);
    } else if (state == 1) {
        const char* labels[3] = {"Letzten Save laden", "Neues Spiel starten", "Zur Kategorieansicht"};
        int idx = activeSelection;
        if (idx < 0) idx = 0;
        if (idx > 2) idx = 2;
        text.draw(176.0f, 150.0f, 0.34f, C2D_Color32(208, 220, 244, 255), "%s", labels[idx]);
    } else if (state != 2) {
        text.draw(176.0f, 146.0f, 0.31f, C2D_Color32(208, 220, 244, 255), "Debug:%s  Ctrl:%s", debugEnabled ? "AN" : "AUS", controlsSwapped ? "Swap" : "Normal");
    }

    text.draw(176.0f, 188.0f, 0.33f, C2D_Color32(168, 184, 216, 255), "Bottom: Aktionen / Auswahl");
    text.draw(176.0f, 208.0f, 0.33f, C2D_Color32(168, 184, 216, 255), "A/Y: Bestaetigen   B: Zurueck");
}

void drawMainMenuHomeView(TextRenderer& text, int selection) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));

    const char* labels[3] = {"Spielen", "Optionen", "Spiel beenden"};
    for (int i = 0; i < 3; ++i) {
        u32 c = selection == i ? C2D_Color32(84, 140, 220, 255) : (i == 2 ? C2D_Color32(96, 50, 50, 255) : C2D_Color32(56, 72, 104, 255));
        C2D_DrawRectSolid(50, 48 + i * 48, 0.0f, 220, 32, c);
        text.draw(78.0f, 58.0f + i * 48, 0.44f, C2D_Color32(245, 245, 255, 255), "%s", labels[i]);
    }
}

void drawMainMenuPlayView(TextRenderer& text, int selection, int selectedSaveSlot, bool hasContinue) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));

    C2D_DrawRectSolid(44, 8, 0.0f, 40, 28, C2D_Color32(56, 72, 104, 255));
    C2D_DrawRectSolid(236, 8, 0.0f, 40, 28, C2D_Color32(56, 72, 104, 255));
    text.draw(58.0f, 16.0f, 0.40f, C2D_Color32(245, 245, 255, 255), "<");
    text.draw(250.0f, 16.0f, 0.40f, C2D_Color32(245, 245, 255, 255), ">");
    text.draw(120.0f, 18.0f, 0.38f, C2D_Color32(192, 205, 235, 255), "Slot %d", selectedSaveSlot);

    u32 c0 = hasContinue ? (selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255)) : C2D_Color32(68, 68, 76, 255);
    u32 c1 = selection == 1 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c2 = selection == 2 ? C2D_Color32(124, 96, 186, 255) : C2D_Color32(70, 58, 102, 255);

    C2D_DrawRectSolid(50, 48, 0.0f, 220, 32, c0);
    C2D_DrawRectSolid(50, 96, 0.0f, 220, 32, c1);
    C2D_DrawRectSolid(50, 144, 0.0f, 220, 32, c2);

    text.draw(72.0f, 58.0f, 0.42f, hasContinue ? C2D_Color32(245, 245, 255, 255) : C2D_Color32(170, 170, 182, 255), hasContinue ? "Fortsetzen" : "Fortsetzen (kein Save)");
    text.draw(86.0f, 106.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Neues Spiel");
    text.draw(112.0f, 154.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Zurueck");
    text.draw(62.0f, 224.0f, 0.32f, C2D_Color32(170, 180, 205, 255), "Links/Rechts: Slot wechseln");
}

void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled, bool controlsSwapped) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
    u32 c0 = selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c1 = selection == 1 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c2 = selection == 2 ? C2D_Color32(186, 116, 74, 255) : C2D_Color32(108, 72, 48, 255);
    u32 c3 = selection == 3 ? C2D_Color32(124, 96, 186, 255) : C2D_Color32(70, 58, 102, 255);

    C2D_DrawRectSolid(40, 42, 0.0f, 240, 32, c0);
    C2D_DrawRectSolid(40, 90, 0.0f, 240, 32, c1);
    C2D_DrawRectSolid(40, 138, 0.0f, 240, 32, c2);
    C2D_DrawRectSolid(40, 186, 0.0f, 240, 32, c3);

    text.draw(52.0f, 50.0f, 0.38f, C2D_Color32(245, 245, 255, 255), "Ingame Debug Info");
    text.draw(226.0f, 50.0f, 0.40f, debugEnabled ? C2D_Color32(120, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), debugEnabled ? "AN" : "AUS");

    text.draw(52.0f, 98.0f, 0.36f, C2D_Color32(245, 245, 255, 255), "Steuerung tauschen");
    text.draw(214.0f, 98.0f, 0.36f, C2D_Color32(210, 220, 240, 255), controlsSwapped ? "AN" : "AUS");

    text.draw(62.0f, 146.0f, 0.34f, C2D_Color32(250, 240, 220, 255), "Kartenfortschritt reset");

    text.draw(132.0f, 194.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Zurueck");
}
