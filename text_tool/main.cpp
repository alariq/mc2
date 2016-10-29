#include "gameos.hpp"
#include "gos_render.h"
#include "gos_font.h"
#include <stdio.h>

// errno + strerror
#include <errno.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "utils/camera.h"
#include "utils/shader_builder.h"
#include "utils/gl_utils.h"

static const uint32_t NUM_GLYPHS = 255 - 32;
static const uint32_t START_GLYPH = 32;

gosGlyphMetrics gm[NUM_GLYPHS];
SDL_Surface* gs[NUM_GLYPHS];

void usage(char** argv) {
    printf("%s <font_name.ttf> <size> <out_font_name>\n", argv[0]);
}

bool gos_save_glyphs(const char* glyphFile, const gosGlyphInfo& gi)
{
    FILE* glyph_info = fopen(glyphFile, "wb");
    if(!glyph_info) {
        int last_err = errno;
        printf("fopen: %s\n", strerror(errno));
        return false;
    }

    fwrite(&gi.num_glyphs_, sizeof(gi.num_glyphs_), 1, glyph_info);
    fwrite(&gi.start_glyph_, sizeof(gi.start_glyph_), 1, glyph_info);

    fwrite(&gi.max_advance_, sizeof(gi.max_advance_), 1, glyph_info);
    fwrite(&gi.font_line_skip_, sizeof(gi.font_line_skip_), 1, glyph_info);

    size_t num_structs_written = 0;

    while(num_structs_written != gi.num_glyphs_) {
        num_structs_written += fwrite(&gi.glyphs_[num_structs_written], 
                sizeof(gosGlyphMetrics), 
                gi.num_glyphs_ - num_structs_written, 
                glyph_info);
    }

    fclose(glyph_info);

    return true;
}

int main(int argc, char** argv)
{
    char* glyphFile = {0};
    char* textureFile = {0};
    const char* tex_ext = ".bmp";
    const char* glyph_ext = ".glyph";

    if(argc < 4) {
        usage(argv);
        return 1;
    }

    const char* const fontFile = argv[1];
    const char* size = argv[2];
    const char* outFile = argv[3];

    int fontSize = atoi(size);
    if(fontSize<=0 || fontSize>=100) {
        printf("Invalid font size: %d\n", fontSize);
        return 1;
    }

    int size_width = 2; // 2 chars enough to store 0 to 99
    textureFile = new char[strlen(outFile) + strlen(tex_ext) + size_width + 1];
    glyphFile = new char[strlen(outFile) + strlen(glyph_ext) + size_width + 1];

    sprintf(textureFile, "%s%d%s", outFile, fontSize, tex_ext);
    sprintf(glyphFile, "%s%d%s", outFile, fontSize, glyph_ext);


    graphics::set_verbose(false);

    graphics::RenderWindowHandle win = graphics::create_window("text_tool", 512, 512);
    if(!win)
        return 1;

    graphics::RenderContextHandle ctx = graphics::init_render_context(win);
    if(!ctx)
        return 1;

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        SPEW(("GLEW", "Error: %s\n", glewGetErrorString(err)));
        return 1;
    }

    SPEW(("GRAPHICS", "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION)));
    if (!GLEW_ARB_vertex_program || !GLEW_ARB_vertex_program)
    {
        SPEW(("GRAPHICS", "No shader program support\n"));
        return 1;
    }

    graphics::make_current_context(ctx, win);

    // do stuff

    if(TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }

    int outlinePixelSize = 0;

    TTF_Font* font = TTF_OpenFont(fontFile, fontSize);
    if(!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(1);
    }

    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, outlinePixelSize);

    // get the loaded font's max height
    const int maxFontHeight = TTF_FontHeight(font);
    const int maxFontAscent = TTF_FontAscent(font);
    const int maxFontDescent = TTF_FontDescent(font);
    const int fontLineSkip = TTF_FontLineSkip(font);
    //TTF_SetFontHinting(font, TTF_HINTING_MONO);

    printf("The font max height is: %d\n", maxFontHeight);
    printf("The font ascent is: %d\n", maxFontAscent);
    printf("The font descent is: %d\n", maxFontDescent);

	printf("family: %s, style: %s, size:%d\n",
            TTF_FontFaceFamilyName(font),
			TTF_FontFaceStyleName(font),
            fontSize);

    int max_advance = 0;
	const SDL_Color fg = { 255, 255, 255, 255 };
	const SDL_Color bg = { 0, 0, 0, 0};

    for(int i=0;i<NUM_GLYPHS;++i) {

        const int glyph_id = i + START_GLYPH;
        gs[i] = TTF_RenderGlyph_Solid(font, glyph_id, fg);
        gm[i].valid = true;

        if(!gs[i]) {
            printf("TTF_RenderGlyph+Blended: %d : %s\n", glyph_id, TTF_GetError());
            gm[i].valid = false;
            continue;
        }

        TTF_GlyphMetrics(font, glyph_id,
                &gm[i].minx, &gm[i].maxx,
                &gm[i].miny, &gm[i].maxy,
                &gm[i].advance);

        max_advance = gm[i].advance > max_advance ? gm[i].advance : max_advance;

    }

    // calculate required texture width and height
    int num_chars_in_line = 32;;
    int fontTextureWidth = num_chars_in_line * max_advance;
    int num_lines = (NUM_GLYPHS + num_chars_in_line - 1) / num_chars_in_line;
    int fontTextureHeight = num_lines * fontLineSkip;

    SDL_Surface* fontTexture = SDL_CreateRGBSurface(0, 
            fontTextureWidth, 
            fontTextureHeight,
            32,
            0x00FF0000,
            0x0000FF00,
            0x000000FF,
            0xFF000000);

    const Uint32 black = SDL_MapRGBA(fontTexture->format, 0,0,0, 0);
    const Uint32 white = SDL_MapRGBA(fontTexture->format, 255,255,255, 255);

    SDL_FillRect(fontTexture, 0, black);

    int x = 0;
    int y = 0;
    for(int i=0;i<NUM_GLYPHS;++i) {

        if(!gm[i].valid)
            continue;

        char buf[256];
        sprintf(buf, "tmp/glyph%d.bmp", i);
        SDL_SaveBMP(gs[i], buf);

        SDL_Rect r;

		r.x = x;// + gm[i].minx;
		r.y = y;// + maxFontAscent - gm[i].maxy;

        //r.x -= outlinePixelSize;
        //r.y -= outlinePixelSize;

        x += max_advance;
        
        if(i>0 && (0 == (i+1) % num_chars_in_line))
        {
            x = 0;
            y += fontLineSkip;
        }

		SDL_BlitSurface(gs[i], 0, fontTexture, &r);

        gm[i].u = r.x;
        gm[i].v = r.y;
    }

    gosGlyphInfo gi;
    gi.num_glyphs_ = NUM_GLYPHS;
    gi.start_glyph_ = START_GLYPH;
    gi.glyphs_ = gm;
    gi.max_advance_ = max_advance;
    gi.font_line_skip_ = fontLineSkip;

    // save glyph information
    gos_save_glyphs(glyphFile, gi);
    
    SDL_SaveBMP(fontTexture, textureFile);

    delete[] textureFile;
    delete[] glyphFile;

    TTF_CloseFont(font);

    TTF_Quit();

    graphics::destroy_render_context(ctx);
    graphics::destroy_window(win);

    return 0;
}

