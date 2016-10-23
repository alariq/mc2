#ifndef GAMEOS_GRAPHICS_H
#define GAMEOS_GRAPHICS_H

namespace graphics {

struct RenderWindow;
struct RenderContext;

typedef RenderWindow*   RenderWindowHandle;
typedef RenderContext*   RenderContextHandle;

void set_verbose(bool is_verbose);

RenderWindowHandle  create_window(const char* pwinname, int width, int height);
RenderContextHandle init_render_context(RenderWindowHandle render_window);

void swap_window(RenderWindowHandle h);
void make_current_context(RenderContextHandle ctx_h, RenderWindowHandle win_h);

void destroy_render_context(RenderContextHandle rc_handle);
void destroy_window(RenderWindowHandle rw_handle);

};

#endif // GAMEOS_GRAPHICS_H


