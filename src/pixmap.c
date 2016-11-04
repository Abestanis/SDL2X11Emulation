#include "X11/Xlib.h"
#include "drawing.h"
#include "errors.h"
#include "resourceTypes.h"
#include "display.h"

Pixmap XCreatePixmap(Display* display, Drawable drawable, unsigned int width, unsigned int height,
                     unsigned int depth) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreatePixmap.html
    SET_X_SERVER_REQUEST(display, XCB_CREATE_PIXMAP);
    (void) drawable;
    // TODO: Adjust masks for depth
    if (width == 0 || height == 0) {
        fprintf(stderr, "Width and/or height are 0 in XCreatePixmap: w = %u, h = %u\n", width, height);
        handleError(0, display, NULL, 0, BadValue, 0);
        return NULL;
    }
    if (depth < 1) {
        fprintf(stderr, "Got unsupported depth (%u) in XCreatePixmap\n", depth);
        handleError(0, display, NULL, 0, BadValue, 0);
        return NULL;
    }
    XID pixmap = malloc(sizeof(XID));
    if (pixmap == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate XID in XCreatePixmap!\n");
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    fprintf(stderr, "%s: addr= %p, w = %d, h = %d\n", __func__, pixmap, width, height);
    GPU_Image* image = GPU_CreateImage((Uint16) width, (Uint16) height, GPU_FORMAT_RGBA);
    if (image == NULL) {
        fprintf(stderr, "GPU_CreateImage failed in XCreatePixmap: %s\n", GPU_PopErrorCode().details);
        free(pixmap);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    if (GPU_LoadTarget(image) == NULL) {
        fprintf(stderr, "GPU_LoadTarget failed in XCreatePixmap: %s\n", GPU_PopErrorCode().details);
        free(pixmap);
        GPU_FreeImage(image);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    fprintf(stderr, "gpu target is %p\n", image->target);
    pixmap->type = PIXMAP;
    pixmap->dataPointer = image;
    return pixmap;
}

void XFreePixmap(Display* display, Pixmap pixmap) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XFreePixmap.html
    SET_X_SERVER_REQUEST(display, XCB_FREE_PIXMAP);
    TYPE_CHECK(pixmap, PIXMAP, display);
    GPU_Image* image = GET_PIXMAP_IMAGE(pixmap);
    free(pixmap);
    if (image->target != NULL) {
        fprintf(stderr, "%s, %d\n", __func__, __LINE__);
        GPU_FreeTarget(image->target);
        image->target = NULL;
    }
    fprintf(stderr, "%s, %d\n", __func__, __LINE__);
    GPU_FreeImage(image);
}
