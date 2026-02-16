#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

void drawBottomInventoryTab(TextRenderer& text) {
    // Aktuell nur Platzhalter-UI: später erweiterbar um echte Items.
    text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Inventar");
    C2D_DrawRectSolid(16, 44, 0.0f, 288, 144, C2D_Color32(28, 34, 46, 255));
    text.draw(44.0f, 108.0f, 0.46f, C2D_Color32(170, 180, 205, 255), "(Aktuell leer)");
}
