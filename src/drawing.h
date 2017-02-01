#ifndef _DRAWING_H_
#define _DRAWING_H_

#include "SDL.h"
#include "resourceTypes.h"
#include "window.h"

#define SDL_SURFACE_DEPTH 32

#define GET_PIXMAP_IMAGE(pixmap) (IS_TYPE(pixmap, PIXMAP) ? ((GPU_Image*) GET_XID_VALUE(pixmap)) : NULL)
#define GET_RENDER_TARGET(drawable, renderer) \
if (IS_TYPE(drawable, WINDOW)) {\
    fprintf(stderr, "getWindowRenderTarget of window %lu in %s.\n", drawable, __func__);\
    renderer = getWindowRenderTarget(drawable);\
} else if (IS_TYPE(drawable, PIXMAP)) {\
    renderer = GET_PIXMAP_IMAGE(drawable)->target;\
} else {\
    fprintf(stderr, "Got unknown drawable type while trying to get renderer in %s, %s, %d\n", __FILE__, __func__, __LINE__);\
    renderer = NULL;\
}
#if SDL_BYTEORDER != SDL_BIG_ENDIAN
#  define GET_RED_FROM_COLOR(color)   ((Uint8) ((color >> 24) & 0xFF))
#  define GET_GREEN_FROM_COLOR(color) ((Uint8) ((color >> 16) & 0xFF))
#  define GET_BLUE_FROM_COLOR(color)  ((Uint8) ((color >> 8) & 0xFF))
#  define GET_ALPHA_FROM_COLOR(color) ((Uint8) (color & 0xFF))
#else
#  define GET_RED_FROM_COLOR(color)   ((Uint8) (color & 0xFF))
#  define GET_GREEN_FROM_COLOR(color) ((Uint8) ((color >> 8) & 0xFF))
#  define GET_BLUE_FROM_COLOR(color)  ((Uint8) ((color >> 16) & 0xFF))
#  define GET_ALPHA_FROM_COLOR(color) ((Uint8) ((color >> 24) & 0xFF))
#endif

GPU_Target* getWindowRenderTarget(Window window);
void flipScreen(void);

#endif /* _DRAWING_H_ */
