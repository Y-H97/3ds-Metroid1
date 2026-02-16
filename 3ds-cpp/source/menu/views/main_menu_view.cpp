#include "main_menu_view.h"
#include "manual_content.h"

#include <citro2d.h>
#include <cstring>

#include "../../ui/text_renderer.h"

namespace {
constexpr int kManualTopVisibleLines = 12;
constexpr float kManualTopStartY = 8.0f;
constexpr float kManualTopLineStep = 18.0f;
constexpr float kManualTopEmptyLineStep = 8.0f;
}

static void drawPanel(float x, float y, float w, float h, u32 bg) {
    C2D_DrawRectSolid(x, y, 0.0f, w, h, bg);
        // Kleiner Helfer für einfarbige UI-Panels.
}

static void drawTopCategories(TextRenderer& text, int state, int activeSelection) {
    int categorySelection = 0;
    if (state == 0) categorySelection = activeSelection;
    else if (state == 1) categorySelection = 0;
    else if (state == 2) categorySelection = 2;
    else categorySelection = 1;
        // Linke Kategorienleiste auf dem Top-Screen.

    const char* categories[4] = {"Spielen", "Handbuch", "Optionen", "Beenden"};
    for (int i = 0; i < 4; ++i) {
        bool selected = (i == categorySelection);
        u32 rowCol = selected ? C2D_Color32(74, 118, 194, 255) : C2D_Color32(42, 56, 84, 255);
        drawPanel(24, 66 + i * 36, 124, 28, rowCol);
        text.draw(34.0f, 74.0f + i * 36, 0.34f, C2D_Color32(238, 242, 250, 255), "%s", categories[i]);
    }
}

static void drawTopManualTextOnly(TextRenderer& text, int activeSelection, int manualScroll, bool useScroll) {
    C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(18, 22, 34, 255));
        // Vollbild-Handbuchansicht auf dem Top-Screen mit optionalem Scroll.

    int topic = activeSelection;
        // Rechte Informationsspalte für eine konkrete Handbuchseite.
    if (topic < 0) topic = 0;
    if (topic >= manualTopicCount()) topic = manualTopicCount() - 1;

    int lineCount = manualLineCount(topic);
    int maxScroll = manualMaxScrollForViewport(topic, kManualTopVisibleLines);
    int scroll = useScroll ? manualScroll : 0;
    if (scroll < 0) scroll = 0;
    if (scroll > maxScroll) scroll = maxScroll;

    float y = kManualTopStartY;
    for (int i = 0; i < kManualTopVisibleLines; ++i) {
        int idx = i + scroll;
        if (idx >= lineCount) break;
        const char* line = manualPageLine(topic, idx);
        bool isEmpty = (line == nullptr || line[0] == '\0');
        if (!isEmpty) {
            u32 textColor = C2D_Color32(224, 234, 252, 255);
            if (std::strstr(line, "ROT ROT = Gefahr") != nullptr) {
                textColor = C2D_Color32(255, 120, 120, 255);
            } else if (std::strstr(line, "GRUEN GRUEN = Raumuebergang") != nullptr) {
                textColor = C2D_Color32(140, 235, 150, 255);
            }
            text.draw(20.0f, y, 0.38f, textColor, "%s", line);
            y += kManualTopLineStep;
        } else {
            y += kManualTopEmptyLineStep;
        }
    }
}

static void drawTopManualPage(TextRenderer& text, int activeSelection, int manualScroll) {
    int topic = activeSelection;
    if (topic < 0) topic = 0;
    if (topic >= manualTopicCount()) topic = manualTopicCount() - 1;
    int lineCount = manualLineCount(topic);
    int maxScroll = manualMaxScrollForViewport(topic, kManualTopVisibleLines);
    int scroll = manualScroll;
    if (scroll < 0) scroll = 0;
    if (scroll > maxScroll) scroll = maxScroll;

    text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "%s", manualTopicName(topic));
    float y = 94.0f;
    for (int i = 0; i < kManualTopVisibleLines; ++i) {
        int idx = i + scroll;
        if (idx >= lineCount) break;
        text.draw(176.0f, y, 0.29f, C2D_Color32(208, 220, 244, 255), "%s", manualPageLine(topic, idx));
        y += 18.0f;
    }

    text.draw(176.0f, 224.0f, 0.28f, C2D_Color32(168, 184, 216, 255), "Scroll %d/%d", scroll, maxScroll);
}

static void drawTopManualList(TextRenderer& text, int activeSelection) {
    int idx = activeSelection;
        // Rechte Informationsspalte für die Handbuch-Themenliste.
    if (idx < 0) idx = 0;
    if (idx > manualTopicCount()) idx = manualTopicCount();
    text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "Handbuch");
    if (idx < manualTopicCount()) {
        text.draw(176.0f, 100.0f, 0.33f, C2D_Color32(208, 220, 244, 255), "%s", manualTopicName(idx));
        text.draw(176.0f, 124.0f, 0.31f, C2D_Color32(182, 198, 230, 255), "A/Y: Thema oeffnen");
        text.draw(176.0f, 144.0f, 0.31f, C2D_Color32(182, 198, 230, 255), "DPad: Thema waehlen");
        text.draw(176.0f, 164.0f, 0.31f, C2D_Color32(182, 198, 230, 255), "Inhalte sind scrollbar.");
    } else {
        text.draw(176.0f, 100.0f, 0.33f, C2D_Color32(208, 220, 244, 255), "Zurueck");
        text.draw(176.0f, 124.0f, 0.31f, C2D_Color32(182, 198, 230, 255), "Geht ins Hauptmenue zurueck.");
    }
}

static void drawTopOptions(TextRenderer& text, int activeSelection, bool debugEnabled, bool controlsSwapped) {
    text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "Einstellungs-Hilfe");
        // Kontext-Hilfe zu den einzelnen Optionen.
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
}

static void drawTopStatus(TextRenderer& text, int selectedSaveSlot, bool hasContinue) {
    text.draw(176.0f, 72.0f, 0.36f, C2D_Color32(238, 242, 250, 255), "Status");
        // Kleiner Statusblock (Slot + Continue-Verfügbarkeit).
    text.draw(176.0f, 96.0f, 0.34f, C2D_Color32(190, 206, 236, 255), "Aktiver Slot: %d", selectedSaveSlot);
    text.draw(176.0f, 120.0f, 0.34f, hasContinue ? C2D_Color32(136, 240, 170, 255) : C2D_Color32(220, 170, 170, 255), hasContinue ? "Fortsetzen verfuegbar" : "Kein Save im Slot");
}

static void drawTopContextLabels(TextRenderer& text, int state, int activeSelection) {
    if (state == 0) {
            // Kontextzeile passend zur aktuellen Auswahl.
        const char* labels[4] = {"Kategorie Spielen", "Kategorie Handbuch", "Kategorie Optionen", "Spiel beenden"};
        int idx = activeSelection;
        if (idx < 0) idx = 0;
        if (idx > 3) idx = 3;
        text.draw(176.0f, 176.0f, 0.32f, C2D_Color32(208, 220, 244, 255), "%s", labels[idx]);
    } else if (state == 1) {
        const char* labels[3] = {"Letzten Save laden", "Neues Spiel starten", "Zur Kategorieansicht"};
        int idx = activeSelection;
        if (idx < 0) idx = 0;
        if (idx > 2) idx = 2;
        text.draw(176.0f, 176.0f, 0.32f, C2D_Color32(208, 220, 244, 255), "%s", labels[idx]);
    }
}

void drawMainMenuTopView(TextRenderer& text, int state, int activeSelection, int selectedSaveSlot, bool hasContinue, bool debugEnabled, bool controlsSwapped, int manualScroll) {
    if (state == 3 || state == 4) {
            // Zentrale Top-View-Komposition abhängig vom Menüzustand.
        drawTopManualTextOnly(text, activeSelection, manualScroll, state == 4);
        return;
    }

    C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(16, 20, 32, 255));
    drawPanel(16, 14, 368, 34, C2D_Color32(30, 40, 62, 255));
    text.draw(26.0f, 24.0f, 0.46f, C2D_Color32(244, 248, 255, 255), "Metroidvania 3DS");
    text.draw(250.0f, 24.0f, 0.34f, C2D_Color32(180, 194, 224, 255), "C++ Core Build");

    drawPanel(16, 58, 140, 166, C2D_Color32(26, 34, 52, 255));
    drawPanel(164, 58, 220, 166, C2D_Color32(32, 42, 64, 255));

    drawTopCategories(text, state, activeSelection);

    if (state == 4) {
        drawTopManualPage(text, activeSelection, manualScroll);
    } else if (state == 3) {
        drawTopManualList(text, activeSelection);
    } else if (state == 2) {
        drawTopOptions(text, activeSelection, debugEnabled, controlsSwapped);
    } else {
        drawTopStatus(text, selectedSaveSlot, hasContinue);
    }

    drawTopContextLabels(text, state, activeSelection);

    text.draw(176.0f, 188.0f, 0.33f, C2D_Color32(168, 184, 216, 255), "Bottom: Aktionen / Auswahl");
    text.draw(176.0f, 208.0f, 0.33f, C2D_Color32(168, 184, 216, 255), "A/Y: Bestaetigen   B: Zurueck");
}

void drawMainMenuHomeView(TextRenderer& text, int selection) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
        // Home-Kategorien als große Touch-/Button-Flächen.

    const char* labels[4] = {"Spielen", "Handbuch", "Optionen", "Spiel beenden"};
    for (int i = 0; i < 4; ++i) {
        u32 c = selection == i ? C2D_Color32(84, 140, 220, 255) : (i == 3 ? C2D_Color32(96, 50, 50, 255) : C2D_Color32(56, 72, 104, 255));
        C2D_DrawRectSolid(50, 48 + i * 48, 0.0f, 220, 32, c);
        text.draw(78.0f, 58.0f + i * 48, 0.44f, C2D_Color32(245, 245, 255, 255), "%s", labels[i]);
    }
}

void drawMainMenuPlayView(TextRenderer& text, int selection, int selectedSaveSlot, bool hasContinue) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
        // Play-Menü mit Slot-Wechsel und Start-/Continue-Aktionen.

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

void drawMainMenuManualListView(TextRenderer& text, int selection) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
        // Liste aller Handbuchthemen plus Zurück-Eintrag.

    for (int i = 0; i < manualTopicCount(); ++i) {
        u32 c = selection == i ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
        C2D_DrawRectSolid(20, 40 + i * 36, 0.0f, 280, 32, c);
        text.draw(28.0f, 50.0f + i * 36, 0.34f, C2D_Color32(245, 245, 255, 255), "%s", manualTopicName(i));
    }

    u32 backCol = selection == manualTopicCount() ? C2D_Color32(124, 96, 186, 255) : C2D_Color32(70, 58, 102, 255);
    C2D_DrawRectSolid(20, 220, 0.0f, 280, 18, backCol);
    text.draw(126.0f, 222.0f, 0.30f, C2D_Color32(245, 245, 255, 255), "Zurueck");
}

void drawMainMenuManualPageControlsView(TextRenderer& text, int topicSelection, int scroll, int maxScroll) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
        // Steuer-Hinweise für Scrollen/Zurück im Handbuchmodus.
    int topic = topicSelection;
    if (topic < 0) topic = 0;
    if (topic >= manualTopicCount()) topic = manualTopicCount() - 1;

    C2D_DrawRectSolid(12, 12, 0.0f, 296, 30, C2D_Color32(56, 72, 104, 255));
    text.draw(18.0f, 20.0f, 0.34f, C2D_Color32(245, 245, 255, 255), "%s", manualTopicName(topic));

    C2D_DrawRectSolid(12, 52, 0.0f, 296, 130, C2D_Color32(36, 46, 66, 255));
    text.draw(22.0f, 64.0f, 0.32f, C2D_Color32(208, 220, 244, 255), "Oben/unten tippen oder DPad");
    text.draw(22.0f, 84.0f, 0.32f, C2D_Color32(208, 220, 244, 255), "fuer Scroll im Handbuchtext.");
    text.draw(22.0f, 108.0f, 0.32f, C2D_Color32(182, 198, 230, 255), "A/Y oder B: Zur Themenliste");
    text.draw(22.0f, 132.0f, 0.32f, C2D_Color32(182, 198, 230, 255), "Scroll: %d / %d", scroll, maxScroll);

    C2D_DrawRectSolid(20, 192, 0.0f, 280, 20, C2D_Color32(70, 58, 102, 255));
    text.draw(114.0f, 196.0f, 0.32f, C2D_Color32(245, 245, 255, 255), "Zurueck");

    C2D_DrawRectSolid(20, 218, 0.0f, 132, 18, C2D_Color32(56, 72, 104, 255));
    C2D_DrawRectSolid(168, 218, 0.0f, 132, 18, C2D_Color32(56, 72, 104, 255));
    text.draw(56.0f, 220.0f, 0.30f, C2D_Color32(245, 245, 255, 255), "Scroll -");
    text.draw(204.0f, 220.0f, 0.30f, C2D_Color32(245, 245, 255, 255), "Scroll +");
}

void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled, bool controlsSwapped) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
        // Optionen-Menü als interaktive Schaltflächen.
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
