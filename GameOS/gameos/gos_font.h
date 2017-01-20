#ifndef GOS_FONT_H
#define GOS_FONT_H

typedef struct {
    int32_t minx;
    int32_t maxx;
    int32_t miny;
    int32_t maxy;
    int32_t advance;
    uint32_t valid;
    uint32_t u;
    uint32_t v;
} gosGlyphMetrics;

struct gosGlyphInfo {
    gosGlyphInfo():num_glyphs_(0), glyphs_(0) {}
    uint32_t num_glyphs_;
    uint32_t start_glyph_;
    gosGlyphMetrics* glyphs_;
    uint32_t max_advance_;
    uint32_t font_ascent_;
    uint32_t font_line_skip_;
};

bool gos_load_glyphs(const char* glyphFile, gosGlyphInfo& gi);

#endif // GOS_FONT_H
