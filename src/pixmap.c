#include "X11/Xlib.h"
#include "drawing.h"
#include "errors.h"
#include "resourceTypes.h"
#include "display.h"

Pixmap XCreatePixmap(Display* display, Drawable drawable, unsigned int width, unsigned int height,
                     unsigned int depth) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreatePixmap.html
    SET_X_SERVER_REQUEST(display, X_CreatePixmap);
    (void) drawable;
    // TODO: Adjust masks for depth
    if (width == 0 || height == 0) {
        fprintf(stderr, "Width and/or height are 0 in XCreatePixmap: w = %u, h = %u\n", width, height);
        handleError(0, display, None, 0, BadValue, 0);
        return None;
    }
    if (depth < 1) {
        fprintf(stderr, "Got unsupported depth (%u) in XCreatePixmap\n", depth);
        handleError(0, display, None, 0, BadValue, 0);
        return None;
    }
    XID pixmap = ALLOC_XID();
    if (pixmap == None) {
        fprintf(stderr, "Out of memory: Could not allocate XID in XCreatePixmap!\n");
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    fprintf(stderr, "%s: addr= %lu, w = %d, h = %d\n", __func__, pixmap, width, height);
    GPU_Image* image = GPU_CreateImage((Uint16) width, (Uint16) height, GPU_FORMAT_RGBA);
    if (image == NULL) {
        fprintf(stderr, "GPU_CreateImage failed in XCreatePixmap: %s\n", GPU_PopErrorCode().details);
        FREE_XID(pixmap);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    if (GPU_LoadTarget(image) == NULL) {
        fprintf(stderr, "GPU_LoadTarget failed in XCreatePixmap: %s\n", GPU_PopErrorCode().details);
        FREE_XID(pixmap);
        GPU_FreeImage(image);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    fprintf(stderr, "gpu target is %p\n", image->target);
    SET_XID_TYPE(pixmap, PIXMAP);
    SET_XID_VALUE(pixmap, image);
    return pixmap;
}

int XFreePixmap(Display* display, Pixmap pixmap) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XFreePixmap.html
    SET_X_SERVER_REQUEST(display, X_FreePixmap);
    TYPE_CHECK(pixmap, PIXMAP, display, 0);
    GPU_Image* image = GET_PIXMAP_IMAGE(pixmap);
    FREE_XID(pixmap);
    if (image->target != NULL) {
        fprintf(stderr, "%s, %d\n", __func__, __LINE__);
        GPU_FreeTarget(image->target);
        image->target = NULL;
    }
    fprintf(stderr, "%s, %d\n", __func__, __LINE__);
    GPU_FreeImage(image);
    return 1;
}

Pixmap XCreateBitmapFromData(Display* display, Drawable d, _Xconst char* data,
                             unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/utilities/XCreateBitmapFromData.html
    SET_X_SERVER_REQUEST(display, X_CreatePixmap);
    XID pixmap = ALLOC_XID();
    if (pixmap == None) {
        fprintf(stderr, "Out of memory: Could not allocate XID in %s!\n", __func__);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void *) data, width, height, 1,
                                                    (int) ceilf(width / 8.0f), 0, 0, 0, 1);
    if (surface == NULL) {
        FREE_XID(pixmap);
        fprintf(stderr, "SDL_CreateRGBSurfaceFrom failed in %s: %s\n", __func__, SDL_GetError());
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    GPU_Image* image = GPU_CopyImageFromSurface(surface);
    SDL_FreeSurface(surface);
    if (image == NULL) {
        fprintf(stderr, "GPU_CreateImage failed in %s: %s\n", __func__, GPU_PopErrorCode().details);
        FREE_XID(pixmap);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    if (GPU_LoadTarget(image) == NULL) {
        fprintf(stderr, "GPU_LoadTarget failed in %s: %s\n", __func__, GPU_PopErrorCode().details);
        FREE_XID(pixmap);
        GPU_FreeImage(image);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    SET_XID_TYPE(pixmap, PIXMAP);
    SET_XID_VALUE(pixmap, image);
    return pixmap;
}
