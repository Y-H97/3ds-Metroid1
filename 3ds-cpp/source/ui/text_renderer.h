#pragma once

#include <3ds.h>
#include <citro2d.h>
#include <cstddef>

class TextRenderer {
public:
    bool init(size_t capacity = 4096);
    void beginFrame();
    void draw(float x, float y, float scale, u32 color, const char* fmt, ...);
    void shutdown();

private:
    C2D_TextBuf buf = nullptr;
};
