#ifndef _DRAWING_H_
#define _DRAWING_H_

#include "SDL.h"
#include "colors.h"
#include "resourceTypes.h"
#include "window.h"

#define SDL_SURFACE_DEPTH 32

#define GET_PIXMAP_IMAGE(pixmap) (IS_TYPE(pixmap, PIXMAP) ? ((GPU_Image*) GET_XID_VALUE(pixmap)) : NULL)
#define GET_RENDER_TARGET(drawable, renderer) \
if (IS_TYPE(drawable, WINDOW)) {\
    LOG("getWindowRenderTarget of window %lu in %s.\n", drawable, __func__);\
    renderer = getWindowRenderTarget(drawable);\
} else if (IS_TYPE(drawable, PIXMAP)) {\
    renderer = GET_PIXMAP_IMAGE(drawable)->target;\
} else {\
    LOG("Got unknown drawable type while trying to get renderer in %s, %s, %d\n",\
        __FILE__, __func__, __LINE__);\
    renderer = NULL;\
}

GPU_Target* getWindowRenderTarget(Window window);
void flipScreen(void);

#endif /* _DRAWING_H_ */
