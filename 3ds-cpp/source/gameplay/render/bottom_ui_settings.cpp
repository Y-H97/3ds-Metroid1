#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

void drawBottomSettingsTab(
    TextRenderer& text,
    int settingsSelection,
    bool showFpsEnabled
) {
    // Einfache Einstellungsseite: FPS-Toggle + Rückkehr ins Hauptmenü.
    text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Einstellungen");
    u32 s0 = settingsSelection == 0 ? C2D_Color32(76, 112, 182, 255) : C2D_Color32(46, 62, 92, 255);
    u32 s1 = settingsSelection == 1 ? C2D_Color32(182, 84, 84, 255) : C2D_Color32(92, 46, 46, 255);
    C2D_DrawRectSolid(20, 54, 0.0f, 280, 42, s0);
    C2D_DrawRectSolid(20, 104, 0.0f, 280, 42, s1);
    text.draw(28.0f, 67.0f, 0.40f, C2D_Color32(236, 236, 245, 255), "FPS Anzeige");
    text.draw(248.0f, 67.0f, 0.44f, showFpsEnabled ? C2D_Color32(130, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), showFpsEnabled ? "AN" : "AUS");
    text.draw(28.0f, 117.0f, 0.39f, C2D_Color32(245, 236, 236, 255), "Zum Hauptmenue");
}
