#include "main_menu_view.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

void drawMainMenuTopView(TextRenderer& text) {
    C2D_DrawRectSolid(0, 0, 0.0f, 400, 240, C2D_Color32(16, 20, 32, 255));
    C2D_DrawRectSolid(36, 52, 0.0f, 328, 136, C2D_Color32(32, 40, 60, 255));
    text.draw(88.0f, 84.0f, 0.75f, C2D_Color32(245, 245, 255, 255), "Metroidvania 3DS");
    text.draw(120.0f, 124.0f, 0.42f, C2D_Color32(180, 190, 215, 255), "C++ Core Build");
}

void drawMainMenuMainView(TextRenderer& text, int selection) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
    text.draw(94.0f, 14.0f, 0.50f, C2D_Color32(235, 235, 245, 255), "Hauptmenue");

    u32 c0 = selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c1 = selection == 1 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c2 = selection == 2 ? C2D_Color32(176, 84, 84, 255) : C2D_Color32(96, 50, 50, 255);

    C2D_DrawRectSolid(60, 48, 0.0f, 200, 45, c0);
    C2D_DrawRectSolid(60, 104, 0.0f, 200, 45, c1);
    C2D_DrawRectSolid(60, 160, 0.0f, 200, 45, c2);

    text.draw(106.0f, 62.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Neues Spiel");
    text.draw(98.0f, 118.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Einstellungen");
    text.draw(94.0f, 174.0f, 0.44f, C2D_Color32(250, 236, 236, 255), "Spiel beenden");
}

void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled) {
    C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(26, 28, 38, 255));
    text.draw(84.0f, 14.0f, 0.50f, C2D_Color32(235, 235, 245, 255), "Einstellungen");

    u32 c0 = selection == 0 ? C2D_Color32(84, 140, 220, 255) : C2D_Color32(56, 72, 104, 255);
    u32 c1 = selection == 1 ? C2D_Color32(124, 96, 186, 255) : C2D_Color32(70, 58, 102, 255);

    C2D_DrawRectSolid(40, 60, 0.0f, 240, 50, c0);
    C2D_DrawRectSolid(40, 124, 0.0f, 240, 50, c1);

    text.draw(52.0f, 76.0f, 0.40f, C2D_Color32(245, 245, 255, 255), "Ingame Debug Info");
    text.draw(228.0f, 76.0f, 0.44f, debugEnabled ? C2D_Color32(120, 250, 160, 255) : C2D_Color32(255, 180, 180, 255), debugEnabled ? "AN" : "AUS");
    text.draw(132.0f, 140.0f, 0.44f, C2D_Color32(245, 245, 255, 255), "Zurueck");
}
