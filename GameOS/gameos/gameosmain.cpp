#include "gameos.hpp"
#include "gameos_graphics.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    // fills in Environment structure
    GetGameOSEnvironment("");

    int w = Environment.screenWidth;
    int h = Environment.screenHeight;

    graphics::RenderWindowHandle win = graphics::create_window("mc2", w, h);
    if(!win)
        return 1;

    graphics::RenderContextHandle ctx = graphics::init_render_context(win);
    if(!ctx)
        return 1;

    Environment.InitializeGameEngine();

    while(true)
    {
        Environment.DoGameLogic();
        Environment.UpdateRenderers();
    }
    
    Environment.TerminateGameEngine();

    graphics::destroy_render_context(ctx);
    graphics::destroy_window(win);

    return 0;
}

