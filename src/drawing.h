#ifndef _DRAWING_H_
#define _DRAWING_H_

#include "SDL.h"
#include "window.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN || 1
#  define DEFAULT_RED_MASK   0xFF000000
#  define DEFAULT_GREEN_MASK 0x00FF0000
#  define DEFAULT_BLUE_MASK  0x0000FF00
#  define DEFAULT_ALPHA_MASK 0x000000FF
#else
#  define DEFAULT_RED_MASK   0x000000FF
#  define DEFAULT_GREEN_MASK 0x0000FF00
#  define DEFAULT_BLUE_MASK  0x00FF0000
#  define DEFAULT_ALPHA_MASK 0xFF000000
#endif
#define SDL_SURFACE_DEPTH 32

#define LOCK_SURFACE(surface)   if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface)
#define UNLOCK_SURFACE(surface) if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface)
#define GET_PIXMAP_TEXTURE(pixmap) (IS_TYPE(pixmap, PIXMAP) ? ((SDL_Texture*) (pixmap)->dataPointer) : NULL)
#define GET_RENDERER(drawable, renderer) \
if (IS_TYPE(drawable, WINDOW)) {\
    renderer = getWindowRenderer(drawable);\
} else if (IS_TYPE(drawable, PIXMAP)) {\
    renderer = GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlRenderer;\
    if (SDL_SetRenderTarget(renderer, GET_PIXMAP_TEXTURE(drawable)) != 0) {\
        fprintf(stderr, "SDL_SetRenderTarget failed while trying to get renderer in %s, %s, %d: %s\n", __FILE__, __func__, __LINE__, SDL_GetError());\
    }\
} else {\
    fprintf(stderr, "Got unknown drawable type while trying to get renderer in %s, %s, %d\n", __FILE__, __func__, __LINE__);\
}

#ifdef DEBUG_WINDOWS
void drawWindowDebugView();
void drawDebugWindowSurfacePlanes();
#endif
void putPixel(SDL_Surface *surface, unsigned int x, unsigned int y, Uint32 pixel);
Uint32 getPixel(SDL_Surface *surface, unsigned int x, unsigned int y);
SDL_Renderer* getWindowRenderer(Window window);
void drawWindowDataToScreen(void);

#endif /* _DRAWING_H_ */
