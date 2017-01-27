#ifndef GAMEOS_GRAPHICS_H
#define GAMEOS_GRAPHICS_H

namespace graphics {

struct RenderWindow;
struct RenderContext;

typedef RenderWindow*   RenderWindowHandle;
typedef RenderContext*   RenderContextHandle;

void set_verbose(bool is_verbose);

RenderWindowHandle  create_window   (const char* pwinname, int width, int height);
bool                resize_window   (RenderWindowHandle rw_handle, int width, int height);
void                get_window_size (RenderWindowHandle rw_handle, int* width, int* height);
void                swap_window     (RenderWindowHandle h);
void                destroy_window  (RenderWindowHandle rw_handle);
bool                is_mode_supported(int width, int height, int bpp);

RenderContextHandle init_render_context     (RenderWindowHandle render_window);
void                make_current_context    (RenderContextHandle ctx_h);
void                destroy_render_context  (RenderContextHandle rc_handle);

};

#endif // GAMEOS_GRAPHICS_H


