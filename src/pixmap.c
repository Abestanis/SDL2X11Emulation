#include "X11/Xlib.h"
#include "drawing.h"
#include "errors.h"
#include "resourceTypes.h"
#include "window.h"

Pixmap XCreatePixmap(Display* display, Drawable drawable, unsigned int width, unsigned int height,
                     unsigned int depth) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreatePixmap.html
    // TODO: Adjust masks for depth
    if (width == 0 || height == 0) {
        fprintf(stderr, "Width and/or height are 0 in XCreatePixmap: w = %u, h = %u\n", width, height);
        handleError(0, display, NULL, 0, BadValue, XCB_CREATE_PIXMAP, 0);
        return NULL;
    }
    if (depth < 1) {
        fprintf(stderr, "Got unsupported depth (%u) in XCreatePixmap\n", depth);
        handleError(0, display, NULL, 0, BadValue, XCB_CREATE_PIXMAP, 0);
        return NULL;
    }
    XID pixmap = malloc(sizeof(XID));
    if (pixmap == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate XID in XCreatePixmap!\n");
        handleOutOfMemory(0, display, 0, XCB_CREATE_PIXMAP, 0);
        return NULL;
    }
    fprintf(stderr, "%s: addr= %p, w = %d, h = %d\n", __func__, pixmap, width, height);
    SDL_Texture* texture = SDL_CreateTexture(GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlRenderer,
                                             SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                             (int) width, (int) height);
    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTexture failed in XCreatePixmap: %s\n", SDL_GetError());
        handleOutOfMemory(0, display, 0, XCB_CREATE_PIXMAP, 0);
        free(pixmap);
        return NULL;
    }
    pixmap->type = PIXMAP;
    pixmap->dataPointer = texture;
    SDL_Renderer* renderer;
    GET_RENDERER(pixmap, renderer);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    return pixmap;
}

void XFreePixmap(Display* display, Pixmap pixmap) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XFreePixmap.html
    TYPE_CHECK(pixmap, PIXMAP, XCB_FREE_PIXMAP, display, );
    SDL_Texture* texture = GET_PIXMAP_TEXTURE(pixmap);
    free(pixmap);
    SDL_DestroyTexture(texture);
}
