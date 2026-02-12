#include "text_renderer.h"

#include <cstdarg>
#include <cstdio>

bool TextRenderer::init(size_t capacity) {
    buf = C2D_TextBufNew(capacity);
    return buf != nullptr;
}

void TextRenderer::beginFrame() {
    if (buf) C2D_TextBufClear(buf);
}

void TextRenderer::draw(float x, float y, float scale, u32 color, const char* fmt, ...) {
    if (!buf || !fmt) return;

    char line[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    C2D_Text text;
    C2D_TextParse(&text, buf, line);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor, x, y, 0.3f, scale, scale, color);
}

void TextRenderer::shutdown() {
    if (buf) {
        C2D_TextBufDelete(buf);
        buf = nullptr;
    }
}
