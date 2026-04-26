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
    gosGlyphInfo()
        : num_glyphs_(0), start_glyph_(0), glyphs_(0),
          max_advance_(0), font_ascent_(0), font_line_skip_(0),
          ink_top_(0), ink_bot_(0), ink_valid_(0) {}
    uint32_t num_glyphs_;
    uint32_t start_glyph_;
    gosGlyphMetrics* glyphs_;
    uint32_t max_advance_;
    uint32_t font_ascent_;
    uint32_t font_line_skip_;

    // Per-glyph ink bounds, runtime-only (NOT serialized — extending
    // gosGlyphMetrics would break gos_load_glyphs's blob-read of legacy
    // .glyph files). Allocated only by the D3F load path; NULL for
    // legacy .bmp+.glyph fonts. Values are int8 offsets relative to
    // the rendered quad top (i.e. relative to atlas row top_trim, not
    // raw atlas row 0). Negative ink_top is legitimate for extended
    // glyphs that sit above the ASCII-trimmed band. ink_valid_[c] is
    // 1 if the glyph has any opaque pixels, 0 otherwise (sentinel —
    // can't use ink_top==ink_bot==0 since a real glyph may legitimately
    // span a single row at offset 0).
    int8_t*  ink_top_;
    int8_t*  ink_bot_;
    uint8_t* ink_valid_;
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
