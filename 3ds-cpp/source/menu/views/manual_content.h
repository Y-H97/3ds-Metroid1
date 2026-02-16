#pragma once

// Zentrale Handbuchdaten für das Hauptmenü.
// Neue Themen werden hier ergänzt, ohne die Render-Logik anzufassen.

int manualTopicCount();
const char* manualTopicName(int topic);
int manualLineCount(int topic);
const char* manualPageLine(int topic, int line);
int manualMaxScrollForViewport(int topic, int viewportLines);
