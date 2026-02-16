#pragma once

struct AppSettingsData {
    // Globale Optionen, die zwischen Spielstarts gespeichert werden.
    bool debugEnabled = true;
    bool controlsSwapped = false;
    bool showFps = false;
};

class AppSettings {
public:
    // Lädt Einstellungen aus der SD-Datei in "out".
    static bool load(AppSettingsData& out);
    // Speichert Einstellungen dauerhaft auf SD.
    static bool save(const AppSettingsData& data);
};
