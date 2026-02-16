#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <cstddef>

class TextRenderer {
public:
    // Erstellt den internen Textpuffer für ein Frame.
    bool init(size_t capacity = 4096);
    // Leert den Puffer zu Frame-Beginn, damit alter Text verschwindet.
    void beginFrame();
    // Zeichnet formatierten Text (printf-Stil) an Position x/y.
    void draw(float x, float y, float scale, u32 color, const char* fmt, ...);
    // Gibt Speicher wieder frei.
    void shutdown();

private:
    C2D_TextBuf buf = nullptr;
};
