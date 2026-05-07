#include "gameos.hpp"
#include "gos_render.h"
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include "gos_input.h"

#include "utils/camera.h"
#include "utils/shader_builder.h"
#include "utils/gl_utils.h"
#include "utils/timing.h"

#include <signal.h>

extern graphics::RenderContextHandle gos_GetRenderContext();

extern void gos_SetProjection(int w, int h);
extern void gos_DestroyRenderer();
extern void gos_RendererBeginFrame();
extern void gos_RendererEndFrame();
extern void gos_RendererHandleEvents();
extern void gos_RenderUpdateDebugInput();
extern void gos_RenderEnableDebugDrawCalls();
extern bool gos_RenderGetEnableDebugDrawCalls();

extern bool gosExitGameOS();

extern bool gos_CreateAudio();
extern void gos_UpdateAudio();
extern void gos_DestroyAudio();

static bool g_exit = false;
static bool g_focus_lost = false;
#if 0
static camera g_camera;
#endif

static void handle_key_down( SDL_Keysym* keysym ) {
    switch( keysym->sym ) {
        case SDLK_ESCAPE:
            if(keysym->mod & KMOD_RALT)
                g_exit = true;
            break;
        case 'd':
            if(keysym->mod & KMOD_RALT)
                gos_RenderEnableDebugDrawCalls();
            break;
    }
}

static void set_mouse_capture(bool enabled)
{
    graphics::RenderWindowHandle win = gos_GetWindow();
    //bool fullscreen = graphics::is_window_fullscreen(win);
    SDL_bool capture = (enabled /*&& fullscreen*/) ? SDL_TRUE : SDL_FALSE;
    graphics::grab_window(win, (bool)capture);

    SDL_CaptureMouse(capture);
    SDL_ShowCursor(enabled ? SDL_DISABLE : SDL_ENABLE);
}

static void process_events( void ) {

    input::beginUpdateMouseState();

    SDL_Event event;
    while( SDL_PollEvent( &event ) ) {

        if(g_focus_lost) {
            if(event.type != SDL_WINDOWEVENT || event.window.event != SDL_WINDOWEVENT_FOCUS_GAINED) {
                continue;
            } else {
                g_focus_lost = false;
            }
        }

        switch( event.type ) {
        case SDL_KEYDOWN:
            handle_key_down( &event.key.keysym );
            // fallthrough
        case SDL_KEYUP:
            input::handleKeyEvent(&event);
            break;
        case SDL_QUIT:
            g_exit = true;
            break;
        case SDL_WINDOWEVENT:
            switch(event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    float w = (float)event.window.data1;(void)w;
                    float h = (float)event.window.data2;(void)h;
#if 1
                    SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
                    //glViewport(0, 0, (GLsizei)w, (GLsizei)h);
                    SPEW(("INPUT", "resize event: w: %f h:%f\n", w, h));
                    printf("WINDOW RESIZED");

                    Environment.screenWidth = w;
                    Environment.screenHeight = h;
                    SDL_GL_GetDrawableSize(window, &Environment.drawableWidth, &Environment.drawableHeight);
#endif
                    gos_SetProjection(w, h);
                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    g_focus_lost = true;
                    set_mouse_capture(false);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    g_focus_lost = false;
                    set_mouse_capture(true);
                    break;
            }
            break;
        case SDL_MOUSEMOTION:
            input::handleMouseMotion(&event); 
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            //input::handleMouseButton(&event);
            break;
        case SDL_MOUSEWHEEL:
            input::handleMouseWheel(&event);
            break;
        }
    }

    input::updateMouseState();
    input::updateKeyboardState();
}

extern bool g_disable_quads;

static void draw_screen( void )
{
    g_disable_quads = false;
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glCullFace(GL_FRONT);
    //CHECK_GL_ERROR;
    
	const int viewport_w = Environment.drawableWidth;
	const int viewport_h = Environment.drawableHeight;
    glViewport(0, 0, viewport_w, viewport_h);

    CHECK_GL_ERROR;

    // TODO: reset all states to sane defaults!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    glDepthMask(GL_TRUE);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    gos_RendererBeginFrame();
    Environment.UpdateRenderers();
    gos_RendererEndFrame();

    glUseProgram(0);
    //CHECK_GL_ERROR;
}

extern float frameRate;


#ifndef DISABLE_GAMEOS_MAIN
int main(int argc, char** argv)
{
    //signal(SIGTRAP, SIG_IGN);

    // gather command line
	size_t cmdline_len = 0;
    for(int i=0;i<argc;++i) {
        cmdline_len += strlen(argv[i]);
        cmdline_len += 1; // ' '
    }
    char* cmdline = new char[cmdline_len + 1];
    size_t offset = 0;
    for(int i=0;i<argc;++i) {
        size_t arglen = strlen(argv[i]);
        memcpy(cmdline + offset, argv[i], arglen);
        cmdline[offset + arglen] = ' ';
        offset += arglen + 1;
    }
    cmdline[cmdline_len] = '\0';

    // fills in Environment structure
    GetGameOSEnvironment(cmdline);

    delete[] cmdline;
    cmdline = NULL;

    Environment.InitializeGameEngine();

	timing::init();

    graphics::RenderWindowHandle win = gos_GetWindow();
    graphics::RenderContextHandle ctx = gos_GetRenderContext();

    while( !g_exit ) {

		uint64_t start_tick = timing::gettickcount();

        process_events();

        if(gos_RenderGetEnableDebugDrawCalls()) {
            gos_RenderUpdateDebugInput();
        } else {
            Environment.DoGameLogic();
        }

        win = gos_GetWindow();
        ctx = gos_GetRenderContext();

		gos_RendererHandleEvents();
        gos_UpdateAudio();

        graphics::make_current_context(ctx);
        draw_screen();
        graphics::swap_window(win);

        g_exit |= gosExitGameOS();

		uint64_t end_tick = timing::gettickcount();
		double dt_sec = timing::ticks2sec(end_tick - start_tick);
		frameRate = (float)(1.0 / dt_sec);
    }
    
    Environment.TerminateGameEngine();

    return 0;
}
#endif // DISABLE_GAMEOS_MAIN
