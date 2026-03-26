#pragma once

namespace mainmenu {
namespace layout {

// Beschreibt einen rechteckigen Bereich auf dem Touchscreen.
// Die Menue-Logik nutzt diese Struktur fuer Hit-Tests und Zeichnungsgrenzen.
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

// Prueft, ob ein Touchpunkt innerhalb eines rechteckigen UI-Bereichs liegt.
inline bool contains(const Rect& rect, int px, int py) {
    return px >= rect.x && px < rect.x + rect.w && py >= rect.y && py < rect.y + rect.h;
}

// Position eines Home-Menue-Eintrags. Jeder weitere Eintrag wird um 48 Pixel
// nach unten verschoben, damit alle Buttons gleichmaessig verteilt sind.
inline Rect homeButtonRect(int index) {
    return {50, 48 + index * 48, 220, 32};
}

// Linker Slot-Pfeil auf dem Spielstand-Bildschirm.
inline Rect playSlotLeftRect() {
    return {44, 8, 40, 28};
}

// Rechter Slot-Pfeil auf dem Spielstand-Bildschirm.
inline Rect playSlotRightRect() {
    return {236, 8, 40, 28};
}

// Hauptbutton eines Speicherstands zum Starten oder Fortsetzen.
inline Rect playButtonRect(int index) {
    return {50, 48 + index * 48, 220, 32};
}

// Allgemeiner Optionsbutton im Optionsmenue.
inline Rect optionsButtonRect(int index) {
    return {40, 42 + index * 48, 240, 32};
}

// Klickbereich eines Handbuch-Themas in der Themenliste.
inline Rect manualTopicRect(int index) {
    return {20, 40 + index * 36, 280, 32};
}

// Zurueck-Button in der Themenuebersicht des Handbuchs.
inline Rect manualBackRect() {
    return {20, 220, 280, 18};
}

// Zurueck-Button innerhalb einer einzelnen Handbuchseite.
inline Rect manualPageBackRect() {
    return {20, 192, 280, 20};
}

// Oberer Touchbereich zum Scrollen in einer Handbuchseite.
inline Rect manualPageScrollUpRect() {
    return {0, 0, 320, 48};
}

// Unterer Touchbereich zum Scrollen in einer Handbuchseite.
inline Rect manualPageScrollDownRect() {
    return {0, 192, 320, 48};
}

} // namespace layout
} // namespace mainmenu
