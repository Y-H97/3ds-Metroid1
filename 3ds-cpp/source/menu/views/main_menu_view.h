#pragma once

class TextRenderer;

void drawMainMenuTopView(TextRenderer& text);
void drawMainMenuMainView(TextRenderer& text, int selection);
void drawMainMenuOptionsView(TextRenderer& text, int selection, bool debugEnabled);
