#pragma once

class TextRenderer;

void drawMainMenuTopView(TextRenderer& text, int state, int activeSelection, int selectedSaveSlot, bool hasContinue, bool debugEnabled, bool controlsSwapped);
void drawMainMenuHomeView(TextRenderer& text, int selection);
void drawMainMenuPlayView(TextRenderer& text, int selection, int selectedSaveSlot, bool hasContinue);
void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled, bool controlsSwapped);
