#pragma once

struct AppSettingsData {
    bool debugEnabled = true;
    bool controlsSwapped = false;
    bool showFps = false;
};

class AppSettings {
public:
    static bool load(AppSettingsData& out);
    static bool save(const AppSettingsData& data);
};
