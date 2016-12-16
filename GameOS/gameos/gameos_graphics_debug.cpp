extern bool g_debug_draw_calls;

void gos_RenderUpdateDebugInput() {

    gosASSERT(g_gos_renderer);
    const uint32_t n = g_gos_renderer->getNumDrawCallsToDraw();

    const DWORD key = gos_GetKey()>>8;
    const bool b_shift = gos_GetKeyStatus(KEY_LSHIFT) != KEY_FREE;
    //const bool b_ctrl = gos_GetKeyStatus(KEY_LCONTROL) != KEY_FREE;
    const uint32_t delta = b_shift ? 10 : 1;

    switch(key) {
        case KEY_LEFT:
            g_gos_renderer->setNumDrawCallsToDraw(n >= delta ? n - delta : 0);
            break;
        case KEY_RIGHT:
            g_gos_renderer->setNumDrawCallsToDraw(n + delta);
            break;
        case KEY_B:
            {
                bool b_break = g_gos_renderer->getBreakOnDrawCall();
                g_gos_renderer->setBreakOnDrawCall(!b_break);
                g_gos_renderer->setBreakDrawCall(n);
            }
            break;
        case KEY_R:
            {
                g_gos_renderer->setBreakOnDrawCall(false);
                g_gos_renderer->setNumDrawCallsToDraw(0);
            }
            break;
        case KEY_ESCAPE:
            g_gos_renderer->setBreakOnDrawCall(false);
            g_gos_renderer->setNumDrawCallsToDraw(0);
            g_debug_draw_calls = false;
            break;
    }
}

