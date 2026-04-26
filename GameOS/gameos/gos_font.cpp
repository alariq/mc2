#include<inttypes.h>
#include <stdio.h>
// errno + strerror
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "gameos.hpp"
#include "gos_font.h"

bool gos_load_glyphs(const char* glyphFile, gosGlyphInfo& gi)
{
    FILE* glyph_info = fopen(glyphFile, "rb");
    if(!glyph_info) {
        int last_err = errno;
        SPEW(("fopen: %s\n", strerror(last_err)));
        return false;
    }

    fread(&gi.num_glyphs_, sizeof(gi.num_glyphs_), 1, glyph_info);
    fread(&gi.start_glyph_, sizeof(gi.start_glyph_), 1, glyph_info);

    fread(&gi.max_advance_, sizeof(gi.max_advance_), 1, glyph_info);
    fread(&gi.font_ascent_, sizeof(gi.font_ascent_), 1, glyph_info);
    fread(&gi.font_line_skip_, sizeof(gi.font_line_skip_), 1, glyph_info);

    size_t num_structs_read = 0;

    gi.glyphs_ = new gosGlyphMetrics[gi.num_glyphs_];

    while(num_structs_read!= gi.num_glyphs_) {
        num_structs_read+= fread(&gi.glyphs_[num_structs_read],
                sizeof(gosGlyphMetrics),
                gi.num_glyphs_ - num_structs_read,
                glyph_info);
    }

    fclose(glyph_info);

    // Pre-fold legacy metrics into the renderer's "u/v = actual sample
    // origin" contract. The on-disk .glyph format stores u/v as a cell
    // origin and minx / (font_ascent - maxy) as both screen offset and
    // atlas shift. The renderer no longer adds those into the UV; do
    // the shift here so the legacy path renders pixel-identical.
    //
    // Guards underflow into uint32 u/v with a SPEW so a malformed
    // community .glyph that violates the assumption is visible in the
    // log rather than silently sampling atlas col 0.
    for(uint32_t i = 0; i < gi.num_glyphs_; ++i) {
        gosGlyphMetrics& g = gi.glyphs_[i];
        int32_t shifted_u = (int32_t)g.u + g.minx;
        int32_t shifted_v = (int32_t)g.v + ((int32_t)gi.font_ascent_ - g.maxy);
        if(shifted_u < 0 || shifted_v < 0) {
            SPEW(("glyph pre-fold underflow in %s at idx %u "
                  "(u=%u minx=%d v=%u maxy=%d ascent=%u); clamping to 0\n",
                  glyphFile, i, g.u, g.minx, g.v, g.maxy, gi.font_ascent_));
            if(shifted_u < 0) shifted_u = 0;
            if(shifted_v < 0) shifted_v = 0;
        }
        g.u = (uint32_t)shifted_u;
        g.v = (uint32_t)shifted_v;
    }

    return true;
}


// ----------------------------------------------------------------------------
// .d3f loader (retail FontEdit format)
//
// Two on-disk versions coexist:
//   v1 ("D3DF", 0x46443344) — D3DFontData with embedded RGBA-ish pixels
//   v4 ("D3F4", 0x34463344) — D3DFontData1 with iTextureCount inline
//                             D3DFontTexture blobs
// Both atlases are 8-bit alpha. v4 stores side length per blob (square);
// v1 stores rectangular dwWidth/dwHeight. See devlogs/fonts_d3f_loader_2026-04-25.md
// for the format details and the empirical sig values (the shipped header
// at GameOS/include/font3d.hpp only documents v1).
// ----------------------------------------------------------------------------

namespace {

const uint32_t D3F_SIG_V1 = 0x46443344u; // "D3DF"
const uint32_t D3F_SIG_V4 = 0x34463344u; // "D3F4"

const uint32_t D3F_MAX_ATLAS_DIM = 4096;

// Field-by-field readers. Avoid blob reads so #pragma pack(1) and bool
// width don't cause portability surprises across MSVC / g++.
bool read_u8 (FILE* f, uint8_t*  v) { return fread(v, 1, 1, f) == 1; }
bool read_u32(FILE* f, uint32_t* v) { return fread(v, 4, 1, f) == 1; }
bool read_i32(FILE* f, int32_t*  v) { return fread(v, 4, 1, f) == 1; }
bool read_bytes(FILE* f, void* dst, size_t n) {
    return fread(dst, 1, n, f) == n;
}

void zero_glyphs(gosGlyphInfo& gi) {
    for(uint32_t i = 0; i < gi.num_glyphs_; ++i) {
        gosGlyphMetrics& g = gi.glyphs_[i];
        g.minx = g.maxx = g.miny = g.maxy = 0;
        g.advance = 0;
        g.valid = 0;
        g.u = g.v = 0;
    }
}

// Scan a glyph rect for tightest non-zero alpha rows. Returns false if the
// rect contained no opaque pixels.
bool scan_glyph_v_extent(const uint8_t* atlas, int atlas_w, int atlas_h,
                         int x, int y, int w, int h,
                         int* out_top, int* out_bot)
{
    if(w <= 0 || h <= 0) return false;
    if(x < 0 || y < 0 || x + w > atlas_w || y + h > atlas_h) return false;
    int top = -1, bot = -1;
    for(int row = 0; row < h; ++row) {
        const uint8_t* line = atlas + (y + row) * atlas_w + x;
        for(int col = 0; col < w; ++col) {
            if(line[col]) {
                if(top < 0) top = row;
                bot = row;
                break;
            }
        }
    }
    if(top < 0) return false;
    *out_top = top;
    *out_bot = bot;
    return true;
}

// Derive ascent / line_skip / per-glyph maxy + atlas v-shift from the
// tightest opaque rows across all valid glyphs. D3F doesn't encode a
// baseline, so all glyphs share the same vertical extent in the rendered
// output — top_trim shifts the atlas sample origin so the trimmed band
// aligns with the glyph quad height.
//
// Scan restricted to printable ASCII (0x20-0x7E). Rare extended-ASCII
// slots (accented capitals like Ä Ö Ü at 0xC4/0xD6/0xDC) reach 3-5 px
// higher than ordinary caps in the AgencyFB atlases — letting them
// drive global_top adds permanent blank headroom to every line of UI
// text and visibly pushes ASCII content low. Trade-off: accents on
// extended chars may render slightly clipped, but those chars are
// vanishingly rare in MC2's UI strings.
void calibrate_vertical(gosGlyphInfo& gi, const gosD3FAtlas& atlas,
                        const uint8_t* bX, const uint8_t* bY,
                        const uint8_t* bW, uint32_t font_height)
{
    int global_top = (int)font_height;
    int global_bot = -1;
    const uint32_t scan_lo = 0x20;
    const uint32_t scan_hi = 0x7E;
    for(uint32_t c = scan_lo; c <= scan_hi && c < gi.num_glyphs_; ++c) {
        if(!gi.glyphs_[c].valid) continue;
        int top, bot;
        if(!scan_glyph_v_extent(atlas.pixels, atlas.width, atlas.height,
                                bX[c], bY[c], bW[c], (int)font_height,
                                &top, &bot)) {
            continue;
        }
        if(top < global_top) global_top = top;
        if(bot > global_bot) global_bot = bot;
    }

    int top_trim = (global_bot >= 0) ? global_top : 0;
    int visible_height = (global_bot >= 0)
        ? (global_bot - global_top + 1)
        : (int)font_height;

    if(visible_height < 1) visible_height = 1;
    if(visible_height > (int)font_height) visible_height = (int)font_height;

    for(uint32_t c = 0; c < gi.num_glyphs_; ++c) {
        gosGlyphMetrics& g = gi.glyphs_[c];
        if(!g.valid) continue;
        g.miny = 0;
        g.maxy = visible_height;
        g.v   += (uint32_t)top_trim;
    }

    gi.font_ascent_    = (uint32_t)visible_height;
    gi.font_line_skip_ = (uint32_t)visible_height + 1; // +1 leading

    // Second pass: populate per-glyph ink bounds for the visual-bounds
    // API (gos_TextVisualBounds). Scans ALL 256 slots, not just ASCII —
    // extended glyphs need bounds too, even though they don't drive
    // global_top/global_bot. Stored relative to the rendered quad top
    // (post-shift), so a glyph that sat at atlas band row top_trim has
    // ink_top=0; an extended glyph that reaches above the trim line has
    // a negative ink_top.
    delete[] gi.ink_top_;
    delete[] gi.ink_bot_;
    delete[] gi.ink_valid_;
    gi.ink_top_   = new int8_t[gi.num_glyphs_];
    gi.ink_bot_   = new int8_t[gi.num_glyphs_];
    gi.ink_valid_ = new uint8_t[gi.num_glyphs_];
    memset(gi.ink_top_,   0, gi.num_glyphs_);
    memset(gi.ink_bot_,   0, gi.num_glyphs_);
    memset(gi.ink_valid_, 0, gi.num_glyphs_);

    for(uint32_t c = 0; c < gi.num_glyphs_; ++c) {
        if(!gi.glyphs_[c].valid) continue;
        int top, bot;
        if(!scan_glyph_v_extent(atlas.pixels, atlas.width, atlas.height,
                                bX[c], bY[c], bW[c], (int)font_height,
                                &top, &bot)) {
            continue;
        }
        int rel_top = top - top_trim;
        int rel_bot = bot - top_trim;
        if(rel_top < INT8_MIN) rel_top = INT8_MIN;
        if(rel_top > INT8_MAX) rel_top = INT8_MAX;
        if(rel_bot < INT8_MIN) rel_bot = INT8_MIN;
        if(rel_bot > INT8_MAX) rel_bot = INT8_MAX;
        gi.ink_top_[c]   = (int8_t)rel_top;
        gi.ink_bot_[c]   = (int8_t)rel_bot;
        gi.ink_valid_[c] = 1;
    }
}

bool parse_v4(FILE* f, gosGlyphInfo& gi, gosD3FAtlas& atlas)
{
    // sig already consumed
    char    szFaceName[64];
    int32_t iSize, iWeight, iTextureCount;
    uint8_t bItalic;
    uint32_t dwFontHeight;
    uint8_t bTexture[256], bX[256], bY[256], bW[256];
    int8_t  cA[256], cC[256];

    if(!read_bytes(f, szFaceName, 64))   return false;
    if(!read_i32  (f, &iSize))           return false;
    if(!read_u8   (f, &bItalic))         return false;
    if(!read_i32  (f, &iWeight))         return false;
    if(!read_i32  (f, &iTextureCount))   return false;
    if(!read_u32  (f, &dwFontHeight))    return false;
    if(!read_bytes(f, bTexture, 256))    return false;
    if(!read_bytes(f, bX, 256))          return false;
    if(!read_bytes(f, bY, 256))          return false;
    if(!read_bytes(f, bW, 256))          return false;
    if(!read_bytes(f, cA, 256))          return false;
    if(!read_bytes(f, cC, 256))          return false;

    if(iTextureCount != 1) {
        SPEW(("d3f v4 with iTextureCount=%d not supported (need 1)\n",
              iTextureCount));
        return false;
    }
    if(dwFontHeight == 0 || dwFontHeight > D3F_MAX_ATLAS_DIM) {
        SPEW(("d3f v4 implausible dwFontHeight=%u\n", dwFontHeight));
        return false;
    }

    uint32_t dwSize = 0;
    if(!read_u32(f, &dwSize))                  return false;
    if(dwSize == 0 || dwSize > D3F_MAX_ATLAS_DIM) {
        SPEW(("d3f v4 implausible atlas dim=%u\n", dwSize));
        return false;
    }

    size_t pixel_count = (size_t)dwSize * (size_t)dwSize;
    uint8_t* pixels = new uint8_t[pixel_count];
    if(!read_bytes(f, pixels, pixel_count)) {
        delete[] pixels;
        return false;
    }

    gi.num_glyphs_   = 256;
    gi.start_glyph_  = 0;
    gi.glyphs_       = new gosGlyphMetrics[256];
    gi.font_ascent_  = dwFontHeight;     // overwritten by calibrate_vertical
    gi.font_line_skip_ = dwFontHeight;   // overwritten by calibrate_vertical
    zero_glyphs(gi);

    uint32_t max_adv = 0;
    for(int c = 0; c < 256; ++c) {
        gosGlyphMetrics& g = gi.glyphs_[c];
        int w   = (int)bW[c];
        int pre = (int)cA[c];
        int post= (int)cC[c];
        int adv = pre + w + post;

        // Under the renderer's "u/v = actual sample origin" contract,
        // u/v point at the glyph's pixels in the atlas (bX/bY) and
        // minx/maxx are the signed screen bearing rect.
        g.minx    = pre;
        g.maxx    = pre + w;
        g.miny    = 0;
        g.maxy    = (int32_t)dwFontHeight;
        g.advance = adv;
        g.u       = bX[c];
        g.v       = bY[c];
        g.valid   = (w > 0) ? 1 : 0;

        if(g.valid && (uint32_t)adv > max_adv) max_adv = (uint32_t)adv;
    }
    gi.max_advance_ = max_adv ? max_adv : dwFontHeight;

    atlas.pixels = pixels;
    atlas.width  = (int)dwSize;
    atlas.height = (int)dwSize;

    calibrate_vertical(gi, atlas, bX, bY, bW, dwFontHeight);
    return true;
}

bool parse_v1(FILE* f, gosGlyphInfo& gi, gosD3FAtlas& atlas)
{
    // sig already consumed
    uint32_t dwWidth, dwFontHeight, dwHeight;
    if(!read_u32(f, &dwWidth))        return false;
    if(!read_u32(f, &dwFontHeight))   return false;
    if(!read_u32(f, &dwHeight))       return false;

    if(dwWidth  == 0 || dwWidth  > D3F_MAX_ATLAS_DIM ||
       dwHeight == 0 || dwHeight > D3F_MAX_ATLAS_DIM ||
       dwFontHeight == 0 || dwFontHeight > dwHeight)
    {
        SPEW(("d3f v1 implausible header w=%u h=%u fh=%u\n",
              dwWidth, dwHeight, dwFontHeight));
        return false;
    }

    uint32_t dwX[256], dwY[256], dwWidths[256];
    int32_t  nA[256], nC[256];
    if(!read_bytes(f, dwX,      sizeof(dwX)))      return false;
    if(!read_bytes(f, dwY,      sizeof(dwY)))      return false;
    if(!read_bytes(f, dwWidths, sizeof(dwWidths))) return false;
    if(!read_bytes(f, nA,       sizeof(nA)))       return false;
    if(!read_bytes(f, nC,       sizeof(nC)))       return false;

    size_t pixel_count = (size_t)dwWidth * (size_t)dwHeight;
    uint8_t* pixels = new uint8_t[pixel_count];
    if(!read_bytes(f, pixels, pixel_count)) {
        delete[] pixels;
        return false;
    }

    gi.num_glyphs_   = 256;
    gi.start_glyph_  = 0;
    gi.glyphs_       = new gosGlyphMetrics[256];
    gi.font_ascent_  = dwFontHeight;
    gi.font_line_skip_ = dwFontHeight;
    zero_glyphs(gi);

    // Build a uint8 per-char width view for calibrate_vertical, which
    // expects the v4 layout. v1 widths are uint32 — clamp to byte range
    // since atlas dims are bounded to 4096 and per-char widths are far
    // smaller in practice.
    uint8_t bX[256], bY[256], bW[256];
    uint32_t max_adv = 0;
    for(int c = 0; c < 256; ++c) {
        uint32_t w   = dwWidths[c];
        int      pre = nA[c];
        int      post= nC[c];
        int      adv = pre + (int)w + post;

        gosGlyphMetrics& g = gi.glyphs_[c];
        g.minx    = pre;
        g.maxx    = pre + (int32_t)w;
        g.miny    = 0;
        g.maxy    = (int32_t)dwFontHeight;
        g.advance = adv;
        g.u       = dwX[c];
        g.v       = dwY[c];
        g.valid   = (w > 0) ? 1 : 0;

        if(g.valid && (uint32_t)adv > max_adv) max_adv = (uint32_t)adv;

        bX[c] = (uint8_t)(dwX[c] & 0xFF);
        bY[c] = (uint8_t)(dwY[c] & 0xFF);
        bW[c] = (w > 255) ? 255 : (uint8_t)w;
    }
    gi.max_advance_ = max_adv ? max_adv : dwFontHeight;

    atlas.pixels = pixels;
    atlas.width  = (int)dwWidth;
    atlas.height = (int)dwHeight;

    // Calibrate using the truncated bX/bY view. Atlases over 256px on
    // either axis would mis-locate the scan rect — guard.
    if(dwWidth <= 256 && dwHeight <= 256) {
        calibrate_vertical(gi, atlas, bX, bY, bW, dwFontHeight);
    }
    return true;
}

} // anonymous namespace

bool gos_load_d3f(const char* d3fFile, gosGlyphInfo& gi, gosD3FAtlas& atlas)
{
    atlas.pixels = NULL;
    atlas.width = atlas.height = 0;

    FILE* f = fopen(d3fFile, "rb");
    if(!f) return false;

    uint32_t sig = 0;
    if(!read_u32(f, &sig)) {
        fclose(f);
        return false;
    }

    bool ok = false;
    if(sig == D3F_SIG_V4) {
        ok = parse_v4(f, gi, atlas);
    } else if(sig == D3F_SIG_V1) {
        ok = parse_v1(f, gi, atlas);
    } else {
        SPEW(("d3f: unrecognized signature 0x%08x in %s\n", sig, d3fFile));
    }

    fclose(f);

    if(!ok) {
        delete[] gi.glyphs_;
        gi.glyphs_ = NULL;
        gi.num_glyphs_ = 0;
        delete[] atlas.pixels;
        atlas.pixels = NULL;
        atlas.width = atlas.height = 0;
    }
    return ok;
}
