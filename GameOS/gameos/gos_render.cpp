#include "gos_render.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "utils/logging.h"

// FIXME: think how to make it better when different parts need window
SDL_Window* g_sdl_window = NULL;

namespace graphics {

static bool VERBOSE_VIDEO = true;
static bool VERBOSE_RENDER = true;
static bool VERBOSE_MODES = true;
static bool ENABLE_VSYNC = true;

struct RenderWindow {
    SDL_Window* window_;
    int width_;
    int height_;
};

struct RenderContext {
   SDL_GLContext glcontext_;
   RenderWindow* render_window_;
};

static void PrintRenderer(SDL_RendererInfo * info);


//==============================================================================
void set_verbose(bool is_verbose)
{
    VERBOSE_VIDEO = is_verbose;
    VERBOSE_RENDER = is_verbose;
    VERBOSE_MODES = is_verbose;
}

//==============================================================================
RenderWindow* create_window(const char* pwinname, int width, int height)
{
	int i, j, m, n;
	SDL_DisplayMode fullscreen_mode;
    SDL_Window* window = NULL; 

    if (VERBOSE_VIDEO) {
        n = SDL_GetNumVideoDrivers();
        if (n == 0) {
            fprintf(stderr, "No built-in video drivers\n");
        } else {
            fprintf(stderr, "Built-in video drivers:");
            for (i = 0; i < n; ++i) {
                if (i > 0) {
                    fprintf(stderr, ",");
                }
                fprintf(stderr, " %s", SDL_GetVideoDriver(i));
            }
            fprintf(stderr, "\n");
        }
    }

    // initialize using 0 videodriver
    if (SDL_VideoInit(0) < 0) {
        fprintf(stderr, "Couldn't initialize video driver: %s\n", SDL_GetError());
        return NULL;
    }
    if (VERBOSE_VIDEO) {
        fprintf(stderr, "Video driver: %s\n", SDL_GetCurrentVideoDriver());
    }

    //not really related to video, but let it be here
    if (VERBOSE_VIDEO) {
        printf("SDL revision: %s\n", SDL_GetRevision());

        SDL_version compiled;
        SDL_version linked;

        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        fprintf(stderr, "We compiled against SDL version %d.%d.%d \n",
            compiled.major, compiled.minor, compiled.patch);
        fprintf(stderr, "But we are linking against SDL version %d.%d.%d.\n",
           linked.major, linked.minor, linked.patch);
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // 4x multisampling :P
    // disable, and add as setting later.
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


    // select core profile if needed
    // COMPATIBILITY, ES,...
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, GL_CONTEXT_FLAG_DEBUG_BIT);

    if (VERBOSE_MODES) {
        SDL_DisplayMode mode;
        int bpp;
        Uint32 Rmask, Gmask, Bmask, Amask;

        n = SDL_GetNumVideoDisplays();
        fprintf(stderr, "Number of displays: %d\n", n);
        for (i = 0; i < n; ++i) {
            fprintf(stderr, "Display %d:\n", i);

            SDL_GetDesktopDisplayMode(i, &mode);
            SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask, &Gmask,
                    &Bmask, &Amask);
            fprintf(stderr,
                    "  Current mode: %dx%d@%dHz, %d bits-per-pixel (%s)\n",
                    mode.w, mode.h, mode.refresh_rate, bpp,
                    SDL_GetPixelFormatName(mode.format));
            if (Rmask || Gmask || Bmask) {
                fprintf(stderr, "      Red Mask   = 0x%.8x\n", Rmask);
                fprintf(stderr, "      Green Mask = 0x%.8x\n", Gmask);
                fprintf(stderr, "      Blue Mask  = 0x%.8x\n", Bmask);
                if (Amask)
                    fprintf(stderr, "      Alpha Mask = 0x%.8x\n", Amask);
            }

            /* Print available fullscreen video modes */
            m = SDL_GetNumDisplayModes(i);
            if (m == 0) {
                fprintf(stderr, "No available fullscreen video modes\n");
            } else {
                fprintf(stderr, "  Fullscreen video modes:\n");
                for (j = 0; j < m; ++j) {
                    SDL_GetDisplayMode(i, j, &mode);
                    SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask,
                            &Gmask, &Bmask, &Amask);
                    fprintf(stderr,
                            "    Mode %d: %dx%d@%dHz, %d bits-per-pixel (%s)\n",
                            j, mode.w, mode.h, mode.refresh_rate, bpp,
                            SDL_GetPixelFormatName(mode.format));
                    if (Rmask || Gmask || Bmask) {
                        fprintf(stderr, "        Red Mask   = 0x%.8x\n",
                                Rmask);
                        fprintf(stderr, "        Green Mask = 0x%.8x\n",
                                Gmask);
                        fprintf(stderr, "        Blue Mask  = 0x%.8x\n",
                                Bmask);
                        if (Amask)
                            fprintf(stderr,
                                    "        Alpha Mask = 0x%.8x\n",
                                    Amask);
                    }
                }
            }
        }
    }

    if (VERBOSE_RENDER) {
        SDL_RendererInfo info;

        n = SDL_GetNumRenderDrivers();
        if (n == 0) {
            fprintf(stderr, "No built-in render drivers\n");
        } else {
            fprintf(stderr, "Built-in render drivers:\n");
            for (i = 0; i < n; ++i) {
                SDL_GetRenderDriverInfo(i, &info);
                PrintRenderer(&info);
            }
        }
    }

    SDL_zero(fullscreen_mode);
    switch (/*state->depth*/0) {
        case 8:
            fullscreen_mode.format = SDL_PIXELFORMAT_INDEX8;
            break;
        case 15:
            fullscreen_mode.format = SDL_PIXELFORMAT_RGB555;
            break;
        case 16:
            fullscreen_mode.format = SDL_PIXELFORMAT_RGB565;
            break;
        case 24:
            fullscreen_mode.format = SDL_PIXELFORMAT_RGB24;
            break;
        default:
            fullscreen_mode.format = SDL_PIXELFORMAT_RGB888;
            break;
    }
    //fullscreen_mode.refresh_rate = state->refresh_rate;

    {
        window = SDL_CreateWindow(pwinname ? pwinname : "--", 
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI);

        if (!window) {
            fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
            return NULL;
        }
        SDL_GetWindowSize(window, &width, &height);

        // NULL to use window width and height and display refresh rate
        // only need to set mode if wanted fullscreen
        if (SDL_SetWindowDisplayMode(window, NULL) < 0) {
            fprintf(stderr, "Can't set up display mode: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            return NULL;
        }

        SDL_ShowWindow(window);
    }

    RenderWindow* rw = new RenderWindow();
    rw->window_ = window;
    rw->width_ = width;
    rw->height_ = height;

    g_sdl_window = window;

    return rw;
}

//==============================================================================
void swap_window(RenderWindowHandle h)
{
    RenderWindow* rw = (RenderWindow*)h;
    assert(rw && rw->window_);
    SDL_GL_SwapWindow(rw->window_);
}

//==============================================================================
RenderContextHandle init_render_context(RenderWindowHandle render_window)
{
    RenderWindow* rw = (RenderWindow*)render_window;
    assert(rw && rw->window_);

    SDL_GLContext glcontext = SDL_GL_CreateContext(rw->window_);
    if (!glcontext ) {
        fprintf(stderr, "SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return NULL;
    }

    if (SDL_GL_MakeCurrent(rw->window_, glcontext) < 0) {
        SDL_GL_DeleteContext(glcontext);
        return NULL;
    } 

    if (ENABLE_VSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

    if(VERBOSE_RENDER) {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(0, &mode);
        printf("Current Display Mode:\n");
        printf("Screen BPP: %d\n", SDL_BITSPERPIXEL(mode.format));
        printf("\n");
        printf("Vendor     : %s\n", glGetString(GL_VENDOR));
        printf("Renderer   : %s\n", glGetString(GL_RENDERER));
        printf("Version    : %s\n", glGetString(GL_VERSION));
        const GLubyte* exts = glGetString(GL_EXTENSIONS);
        printf("Extensions : %s\n", exts);
        printf("\n");

        int value;
        int status = 0;

        /*
           status = SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
           if (!status) {
           printf("SDL_GL_RED_SIZE: requested %d, got %d\n", 5, value);
           } else {
           printf("Failed to get SDL_GL_RED_SIZE: %s\n", SDL_GetError());
           }
           status = SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
           if (!status) {
           printf("SDL_GL_GREEN_SIZE: requested %d, got %d\n", 5, value);
           } else {
           printf("Failed to get SDL_GL_GREEN_SIZE: %s\n", SDL_GetError());
           }
           status = SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
           if (!status) {
           printf("SDL_GL_BLUE_SIZE: requested %d, got %d\n", 5, value);
           } else {
           printf("Failed to get SDL_GL_BLUE_SIZE: %s\n", SDL_GetError());
           }
           */
        //status = SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
        //if (!status) {
        //    printf("SDL_GL_DEPTH_SIZE: requested %d, got %d\n", 16, value);
        //} else {
        //    printf("Failed to get SDL_GL_DEPTH_SIZE: %s\n", SDL_GetError());
        //}

		/*
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
        if (!status) {
            printf("SDL_GL_MULTISAMPLEBUFFERS: %d\n", value);
        } else {
            printf("Failed to get SDL_GL_MULTISAMPLEBUFFERS: %s\n",
                    SDL_GetError());
        }

        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
        if (!status) {
            printf("SDL_GL_MULTISAMPLESAMPLES: %d\n", value);
        } else {
            printf("Failed to get SDL_GL_MULTISAMPLESAMPLES: %s\n",
                    SDL_GetError());
        }
		*/
        status = SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
        if (!status) {
            printf("SDL_GL_ACCELERATED_VISUAL: %d\n", value);
        } else {
            printf("Failed to get SDL_GL_ACCELERATED_VISUAL: %s\n",
                    SDL_GetError());
        }
		
        status = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
        if (!status) {
            printf("SDL_GL_CONTEXT_MAJOR_VERSION: %d\n", value);
        } else {
            printf("Failed to get SDL_GL_CONTEXT_MAJOR_VERSION: %s\n", SDL_GetError());
        }

        status = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
        if (!status) {
            printf("SDL_GL_CONTEXT_MINOR_VERSION: %d\n", value);
        } else {
            printf("Failed to get SDL_GL_CONTEXT_MINOR_VERSION: %s\n", SDL_GetError());
        }
    }

    RenderContext* rc = new RenderContext();
    rc->glcontext_ = glcontext;
    rc->render_window_ = render_window;

	return rc;
}

//==============================================================================
void destroy_render_context(RenderContextHandle rc_handle)
{
    RenderContext* rc = (RenderContext*)rc_handle;
    assert(rc);

    SDL_GL_DeleteContext(rc->glcontext_);
    delete rc;
}

//==============================================================================
void make_current_context(RenderContextHandle ctx_h)
{
    RenderContext* rc = (RenderContext*)ctx_h;
    assert(rc && rc->render_window_ && rc->glcontext_);

    RenderWindow* rw = rc->render_window_;
    assert(rw && rw->window_);

    SDL_GL_MakeCurrent(rw->window_, rc->glcontext_);
}

//==============================================================================
bool resize_window(RenderWindowHandle rw_handle, int width, int height)
{
    RenderWindow* rw = (RenderWindow*)rw_handle;
    assert(rw);

    SDL_SetWindowSize(rw->window_, width, height);
    rw->width_ = width;
    rw->height_ = height;

    return true;
}

//==============================================================================
bool set_window_fullscreen(RenderWindowHandle rw_handle, bool fullscreen)
{
    RenderWindow* rw = (RenderWindow*)rw_handle;
    assert(rw);

    Uint32 flags = fullscreen ? /*SDL_WINDOW_FULLSCREEN*/ SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    
    if(0 != SDL_SetWindowFullscreen(rw->window_, flags)) {
        log_error("SDL_SetWindowFullscreen: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

//==============================================================================
bool is_mode_supported(int width, int height, int bpp) {

    int displayIndex = 0;
    //displayIndex = SDL_GetWindowDisplayIndex(win_h);

    SDL_DisplayMode desired;
	desired.format = bpp==16 ? SDL_PIXELFORMAT_RGB565 : SDL_PIXELFORMAT_RGB888;
	desired.w = width;
	desired.h = height;
	desired.refresh_rate = 0;
	desired.driverdata = 0;

    SDL_DisplayMode returned;
    
    if(NULL == SDL_GetClosestDisplayMode(displayIndex, &desired, &returned)) {
        log_error("resize_window: %s\n", SDL_GetError());
        return false;
    }

    //const char* df = SDL_GetPixelFormatName(desired.format);
    //const char* rf = SDL_GetPixelFormatName(returned.format);

    if(returned.w == desired.w && returned.h == desired.h && returned.format == desired.format)
        return true;

    return false;
}

//==============================================================================
int get_window_display_index(RenderContextHandle ctx_h)
{
    RenderContext* rc = (RenderContext*)ctx_h;
    assert(rc);

    RenderWindow* rw = rc->render_window_;
    assert(rw && rw->window_);

    return SDL_GetWindowDisplayIndex(rw->window_);
}

//==============================================================================
int get_num_display_modes(int display_index)
{
    return SDL_GetNumDisplayModes(display_index);
}

//==============================================================================
bool get_desktop_display_mode(int display_index, int* width, int* height, int* bpp)
{
    assert(width && height && bpp);

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(display_index, &dm) != 0) {
        log_error("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return false;
    }

    *width = dm.w;
    *height = dm.h;
    *bpp = SDL_BITSPERPIXEL(dm.format);
    return true;
}

//==============================================================================
bool get_display_mode_by_index(int display_index, int mode_index, int* width, int* height, int* bpp)
{
    assert(width && height && bpp);

    SDL_DisplayMode dm;
    if (SDL_GetDisplayMode(display_index, mode_index, &dm) != 0) {
        log_error("SDL_GetDisplayMode failed: %s", SDL_GetError());
        return false;
    }

    *width = dm.w;
    *height = dm.h;
    *bpp = SDL_BITSPERPIXEL(dm.format);
    return true;
}

//==============================================================================
void get_window_size(RenderWindowHandle rw_handle, int* width, int* height)
{
    RenderWindow* rw = (RenderWindow*)rw_handle;
    assert(rw && width && height);
    *width = rw->width_;
    *height = rw->height_;
}

//==============================================================================
void get_drawable_size(RenderWindowHandle rw_handle, int* width, int* height)
{
    RenderWindow* rw = (RenderWindow*)rw_handle;
    assert(rw && width && height);
	// TOD: does it make sense to cahce this value? probably not
	SDL_GL_GetDrawableSize(rw->window_, width, height);
}

//==============================================================================
void destroy_window(RenderWindowHandle rw_handle)
{
    RenderWindow* rw = (RenderWindow*)rw_handle;
    SDL_DestroyWindow(rw->window_);
    delete rw;

    g_sdl_window = NULL;
}

//==============================================================================
static void PrintRendererFlag(Uint32 flag)
{
	switch (flag) {
	case SDL_RENDERER_PRESENTVSYNC:
		fprintf(stderr, "PresentVSync");
		break;
	case SDL_RENDERER_ACCELERATED:
		fprintf(stderr, "Accelerated");
		break;
	default:
		fprintf(stderr, "0x%8.8x", flag);
		break;
	}
}

//==============================================================================
static void PrintPixelFormat(Uint32 format)
{
	switch (format) {
	case SDL_PIXELFORMAT_UNKNOWN:
		fprintf(stderr, "Unknwon");
		break;
	case SDL_PIXELFORMAT_INDEX1LSB:
		fprintf(stderr, "Index1LSB");
		break;
	case SDL_PIXELFORMAT_INDEX1MSB:
		fprintf(stderr, "Index1MSB");
		break;
	case SDL_PIXELFORMAT_INDEX4LSB:
		fprintf(stderr, "Index4LSB");
		break;
	case SDL_PIXELFORMAT_INDEX4MSB:
		fprintf(stderr, "Index4MSB");
		break;
	case SDL_PIXELFORMAT_INDEX8:
		fprintf(stderr, "Index8");
		break;
	case SDL_PIXELFORMAT_RGB332:
		fprintf(stderr, "RGB332");
		break;
	case SDL_PIXELFORMAT_RGB444:
		fprintf(stderr, "RGB444");
		break;
	case SDL_PIXELFORMAT_RGB555:
		fprintf(stderr, "RGB555");
		break;
	case SDL_PIXELFORMAT_BGR555:
		fprintf(stderr, "BGR555");
		break;
	case SDL_PIXELFORMAT_ARGB4444:
		fprintf(stderr, "ARGB4444");
		break;
	case SDL_PIXELFORMAT_ABGR4444:
		fprintf(stderr, "ABGR4444");
		break;
	case SDL_PIXELFORMAT_ARGB1555:
		fprintf(stderr, "ARGB1555");
		break;
	case SDL_PIXELFORMAT_ABGR1555:
		fprintf(stderr, "ABGR1555");
		break;
	case SDL_PIXELFORMAT_RGB565:
		fprintf(stderr, "RGB565");
		break;
	case SDL_PIXELFORMAT_BGR565:
		fprintf(stderr, "BGR565");
		break;
	case SDL_PIXELFORMAT_RGB24:
		fprintf(stderr, "RGB24");
		break;
	case SDL_PIXELFORMAT_BGR24:
		fprintf(stderr, "BGR24");
		break;
	case SDL_PIXELFORMAT_RGB888:
		fprintf(stderr, "RGB888");
		break;
	case SDL_PIXELFORMAT_BGR888:
		fprintf(stderr, "BGR888");
		break;
	case SDL_PIXELFORMAT_ARGB8888:
		fprintf(stderr, "ARGB8888");
		break;
	case SDL_PIXELFORMAT_RGBA8888:
		fprintf(stderr, "RGBA8888");
		break;
	case SDL_PIXELFORMAT_ABGR8888:
		fprintf(stderr, "ABGR8888");
		break;
	case SDL_PIXELFORMAT_BGRA8888:
		fprintf(stderr, "BGRA8888");
		break;
	case SDL_PIXELFORMAT_ARGB2101010:
		fprintf(stderr, "ARGB2101010");
		break;
	case SDL_PIXELFORMAT_YV12:
		fprintf(stderr, "YV12");
		break;
	case SDL_PIXELFORMAT_IYUV:
		fprintf(stderr, "IYUV");
		break;
	case SDL_PIXELFORMAT_YUY2:
		fprintf(stderr, "YUY2");
		break;
	case SDL_PIXELFORMAT_UYVY:
		fprintf(stderr, "UYVY");
		break;
	case SDL_PIXELFORMAT_YVYU:
		fprintf(stderr, "YVYU");
		break;
	default:
		fprintf(stderr, "0x%8.8x", format);
		break;
	}
}

//==============================================================================
static void PrintRenderer(SDL_RendererInfo * info)
{
    size_t i, count;

    fprintf(stderr, "  Renderer %s:\n", info->name);

    fprintf(stderr, "    Flags: 0x%8.8X", info->flags);
    fprintf(stderr, " (");
    count = 0;
    for (i = 0; i < sizeof(info->flags) * 8; ++i) {
        Uint32 flag = (1 << i);
        if (info->flags & flag) {
            if (count > 0) {
                fprintf(stderr, " | ");
            }
            PrintRendererFlag(flag);
            ++count;
        }
    }
    fprintf(stderr, ")\n");

    fprintf(stderr, "    Texture formats (%d): ", info->num_texture_formats);
    for (i = 0; i < info->num_texture_formats; ++i) {
        if (i > 0) {
			fprintf(stderr, ", ");
		}
		PrintPixelFormat(info->texture_formats[i]);
	}
	fprintf(stderr, "\n");

	if (info->max_texture_width || info->max_texture_height) {
		fprintf(stderr, "    Max Texture Size: %dx%d\n",
				info->max_texture_width, info->max_texture_height);
	}
}


}; // namespace graphics
