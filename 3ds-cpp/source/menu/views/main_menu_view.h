#pragma once

class TextRenderer;

void drawMainMenuTopView(TextRenderer& text, int state, int activeSelection, int selectedSaveSlot, bool hasContinue, bool debugEnabled, bool controlsSwapped, int manualScroll);
void drawMainMenuHomeView(TextRenderer& text, int selection);
void drawMainMenuPlayView(TextRenderer& text, int selection, int selectedSaveSlot, bool hasContinue);
void drawMainMenuManualListView(TextRenderer& text, int selection);
void drawMainMenuManualPageControlsView(TextRenderer& text, int topicSelection, int scroll, int maxScroll);
void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled, bool controlsSwapped);
