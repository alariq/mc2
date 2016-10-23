#include "gameos.hpp"
#include "gos_render.h"
#include <stdio.h>

// errno + strerror
#include <errno.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "utils/camera.h"
#include "utils/shader_builder.h"
#include "utils/gl_utils.h"

typedef struct {
	int minx,
		maxx,
		miny,
		maxy,
		advance;
    int valid;    
} GlyphMetrics;

static const uint32_t NUM_GLYPHS = 255;
GlyphMetrics gm[NUM_GLYPHS];
SDL_Surface* gs[NUM_GLYPHS];

void usage(char** argv) {
    printf("%s <font.ttf>\n", argv[0]);
}

int main(int argc, char** argv)
{
    char* fontFile = {0};

    if(argc < 2) {
        usage(argv);
        exit(1);
    } else {
        fontFile = argv[1];
    }

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

    int fontSize = 16;
    int outlinePixelSize = 0;

    // load font.ttf at size 16 into font
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

    FILE* glyph_info = fopen("out.glyph", "wb");
    if(!glyph_info) {
        int last_err = errno;
        printf("fopen: %s\n", strerror(errno));
        exit(last_err);
    }

    fwrite(&NUM_GLYPHS, sizeof(NUM_GLYPHS), 1, glyph_info);

    for(int i=0;i<NUM_GLYPHS;++i) {

        gs[i] = TTF_RenderGlyph_Blended(font, i, fg);
        gm[i].valid = true;

        if(!gs[i]) {
            printf("TTF_RenderGlyph+Blended: %d : %s\n", i, TTF_GetError());
            gm[i].valid = false;
            continue;
        }

        TTF_GlyphMetrics(font, i,
                &gm[i].minx, &gm[i].maxx,
                &gm[i].miny, &gm[i].maxy,
                &gm[i].advance);

        max_advance = gm[i].advance > max_advance ? gm[i].advance : max_advance;

    }

    fwrite(&max_advance, sizeof(max_advance), 1, glyph_info);
    fwrite(&fontLineSkip, sizeof(fontLineSkip), 1, glyph_info);

    size_t num_structs_written = fwrite(&gm[0], sizeof(GlyphMetrics), NUM_GLYPHS, glyph_info);
    while(num_structs_written != NUM_GLYPHS) {
        num_structs_written += fwrite(&gm[num_structs_written], 
                sizeof(GlyphMetrics), 
                NUM_GLYPHS - num_structs_written, 
                glyph_info);
    }

    // calculate required texture width and height
    int total_width = (max_advance * NUM_GLYPHS);
    int fontTextureWidth = 512;
    int num_chars_in_line = (fontTextureWidth + max_advance - 1) / max_advance;
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

		r.x = x + gm[i].minx;
		r.y = y + maxFontAscent - gm[i].maxy;

        //r.x -= outlinePixelSize;
        //r.y -= outlinePixelSize;

        x += max_advance;
        if(0 == (i % num_chars_in_line - 1))
        {
            x = 0;
            y += fontLineSkip;
        }

		SDL_BlitSurface(gs[i], 0, fontTexture, &r);

        fwrite(&r, 2 * sizeof(int), 1, glyph_info);
    }

    fclose(glyph_info);

    SDL_SaveBMP(fontTexture, "out.bmp");

    TTF_CloseFont(font);

    TTF_Quit();

    graphics::destroy_render_context(ctx);
    graphics::destroy_window(win);

    return 0;
}

