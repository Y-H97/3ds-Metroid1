#include "app_settings.h"

#include <cstdio>
#include <sys/stat.h>

constexpr const char* SETTINGS_DIR = "sdmc:/3ds/3ds-cpp";
constexpr const char* SETTINGS_PATH = "sdmc:/3ds/3ds-cpp/settings.dat";

bool AppSettings::load(AppSettingsData& out) {
    // Erwartetes Dateiformat: "debug controls fps" als 0/1-Werte.
    FILE* f = fopen(SETTINGS_PATH, "rb");
    if (!f) return false;

    int debug = 1;
    int controls = 0;
    int fps = 0;
    int got = std::fscanf(f, " %d %d %d", &debug, &controls, &fps);
    fclose(f);
    if (got != 3) return false;

    out.debugEnabled = (debug != 0);
    out.controlsSwapped = (controls != 0);
    out.showFps = (fps != 0);
    return true;
}

bool AppSettings::save(const AppSettingsData& data) {
    // Zielverzeichnisse sicherstellen und dann Werte als Zeile schreiben.
    mkdir("sdmc:/3ds", 0777);
    mkdir(SETTINGS_DIR, 0777);

    FILE* f = fopen(SETTINGS_PATH, "wb");
    if (!f) return false;

    std::fprintf(f, "%d %d %d\n", data.debugEnabled ? 1 : 0, data.controlsSwapped ? 1 : 0, data.showFps ? 1 : 0);
    fclose(f);
    return true;
}
