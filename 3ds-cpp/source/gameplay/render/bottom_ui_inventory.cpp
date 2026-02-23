#include "bottom_ui_tabs.h"

#include <citro2d.h>

#include "../../ui/text_renderer.h"

void drawBottomInventoryTab(TextRenderer& text, uint32_t collectedItems, uint32_t activeItems, int selectionIndex) {
    // Basislayout
    text.draw(12.0f, 12.0f, 0.48f, C2D_Color32(230, 230, 240, 255), "Inventar");
    C2D_DrawRectSolid(16, 44, 0.0f, 288, 144, C2D_Color32(28, 34, 46, 255));

    // Liste der gesammelten Items zusammensetzen
    struct Entry { const char* name; uint32_t flag; bool active; };
    std::vector<Entry> list;
    if (collectedItems & GameplayScene::ITEM_DOUBLE_JUMP) {
        list.push_back({"Doppelsprung", GameplayScene::ITEM_DOUBLE_JUMP, (activeItems & GameplayScene::ITEM_DOUBLE_JUMP) != 0});
    }

    if (list.empty()) {
        text.draw(44.0f, 108.0f, 0.46f, C2D_Color32(170, 180, 205, 255), "(Aktuell leer)");
        return;
    }

    // Zeichne Einträge (ein pro Zeile)
    float y = 54.0f;
    for (size_t i = 0; i < list.size(); ++i) {
        const Entry& e = list[i];
        bool sel = (int)i == selectionIndex;
        if (sel) {
            // Markierung hinterlegen
            C2D_DrawRectSolid(18, y - 2, 0.0f, 268, 20, C2D_Color32(50, 50, 70, 255));
        }
        text.draw(24.0f, y, 0.46f, C2D_Color32(235, 235, 245, 255), "%s: %s",
                  e.name,
                  e.active ? "ON" : "OFF");
        y += 24.0f;
    }
}
