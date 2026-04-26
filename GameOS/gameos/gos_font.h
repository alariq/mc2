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

// Atlas extracted from a .d3f file. Pixels are 8-bit alpha, square or
// rectangular per format version. Caller takes ownership and must
// delete[] pixels.
struct gosD3FAtlas {
    uint8_t* pixels;
    int      width;
    int      height;
};

bool gos_load_glyphs(const char* glyphFile, gosGlyphInfo& gi);

// Loads a retail .d3f font (v1 or v4). On success, fills `gi` with
// per-glyph metrics + global ascent/line_skip derived via alpha-scan,
// and hands the embedded 8-bit alpha atlas back through `atlas`.
// Returns false (without allocating) if the file is missing or has an
// unrecognized signature.
bool gos_load_d3f(const char* d3fFile, gosGlyphInfo& gi, gosD3FAtlas& atlas);

#endif // GOS_FONT_H
