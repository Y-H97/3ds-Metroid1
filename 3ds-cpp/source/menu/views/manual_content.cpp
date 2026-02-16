#include "manual_content.h"

namespace {

const char* const kManualTopics[] = {
    "Steuerung: Standard",
    "Steuerung: Getauscht",
    "Speichern",
    "Gefahr-Tiles",
    "Transition-Tiles",
};

const char* const kManualPages[][16] = {
    {
        "Standard-Schema (Empfohlen)",
        "",
        "TOP Screen:",
        "- Schieben: Circle Pad links/rechts",
        "- Springen: A",
        "",
        "BOTTOM Screen:",
        "- Tabs wechseln: DPad links/rechts",
        "- Menues: DPad hoch/runter",
        "- Bestaetigen in Menues: Y",
        "",
        "Hinweis:",
        "- Select bringt dich ins Hauptmenue.",
        nullptr
    },
    {
        "Getauschtes Schema", "",
        "TOP Screen:",
        "- Schieben: DPad links/rechts",
        "- Springen: A",
        "",
        "BOTTOM Screen:",
        "- Tabs wechseln: Circle Pad links/rechts",
        "- Menues: Circle Pad hoch/runter",
        "- Bestaetigen in Menues: Y",
        "",
        "Aktivierung:",
        "- Hauptmenue > Optionen > Steuerung",
        nullptr
    },
    {
        "Speichern & Fortsetzen", "",
        "- Es gibt 3 getrennte Save-Slots.",
        "- Slot waehlst du im Menue 'Spielen'.",
        "",
        "Autosave:",
        "- Beim Erreichen eines Checkpoints.",
        "- Beim Schliessen wird der Stand gesichert.",
        "",
        "Fortsetzen:",
        "- Laedt den letzten Stand des aktiven Slots.",
        "",
        "Kartenfortschritt:",
        "- Wird pro Slot getrennt gespeichert.",
        nullptr
    },
    {
        "Gefahr-Tiles", "",
        "- Gefahr-Tiles (Tile-ID 2).",
        "- Beruehrung fuehrt sofort zum Respawn.",
        "",
        "Optik-Beispiel:",
        "- ROT ROT = Gefahr / toedlich",
        "",
        "Respawn:",
        "- Erfolgt am letzten Checkpoint.",
        "- Wenn keiner aktiv: sichere Position.",
        "",
        "Tipp:",
        "- Nutze Checkpoints vor riskanten Raeumen.",
        nullptr
    },
    {
        "Transition-Tiles", "",
        "- Transition-Tiles (Tile-ID 3).",
        "- Sie verbinden Raeume miteinander.",
        "",
        "Optik-Beispiel:",
        "- GRUEN GRUEN = Raumuebergang",
        "",
        "Ablauf:",
        "- Beim Betreten wird der Zielraum geladen.",
        "- Position wird an Eingangskante gesetzt.",
        "- Bei Checkpoint-Raeumen wird gespeichert.",
        "",
        "Map:",
        "- Neue besuchte Zellen werden sichtbar.",
        nullptr
    },
};

constexpr int kTopicCount = static_cast<int>(sizeof(kManualTopics) / sizeof(kManualTopics[0]));

bool validTopic(int topic) {
    return topic >= 0 && topic < kTopicCount;
}

} // namespace

int manualTopicCount() {
    // Anzahl verfügbarer Handbuchthemen.
    return kTopicCount;
}

const char* manualTopicName(int topic) {
    // Titel eines Themas anhand seines Index.
    if (!validTopic(topic)) return "";
    return kManualTopics[topic];
}

int manualLineCount(int topic) {
    // Zählt Zeilen bis zum nullptr-Sentinel.
    if (!validTopic(topic)) return 0;
    int count = 0;
    while (count < 16 && kManualPages[topic][count] != nullptr) {
        count++;
    }
    return count;
}

const char* manualPageLine(int topic, int line) {
    // Liefert eine konkrete Zeile einer Handbuchseite.
    if (!validTopic(topic)) return "";
    if (line < 0 || line >= 16) return "";
    const char* value = kManualPages[topic][line];
    return value ? value : "";
}

int manualMaxScrollForViewport(int topic, int viewportLines) {
    // Wie weit kann gescrollt werden, damit alle Zeilen sichtbar werden.
    int count = manualLineCount(topic);
    if (viewportLines <= 0) return 0;
    int maxScroll = count - viewportLines;
    return maxScroll > 0 ? maxScroll : 0;
}
