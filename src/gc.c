#include "gc.h"
#include "display.h"


int XFreeGC(Display* display, GC gc) {
    SET_X_SERVER_REQUEST(display, X_FreeGC);
    GraphicContext* gContext = GET_GC(gc);
    if (gContext->stipple != None) {
        XFreePixmap(display, gContext->stipple);
    }
    free(gContext);
    FREE_XID(gc->gid);
    free(gc);
    return 1;
}

GC XCreateGC(Display* display, Drawable d, unsigned long valuemask, XGCValues* values) {
    // https://tronche.com/gui/x/xlib/GC/XCreateGC.html
    SET_X_SERVER_REQUEST(display, X_CreateGC);
    TYPE_CHECK(d, DRAWABLE, display, NULL);
    if (IS_TYPE(d, WINDOW) && IS_INPUT_ONLY(d)) {
        handleError(0, display, d, 0, BadMatch, 0);
        return NULL;
    }
    XID contextId = ALLOC_XID();
    if (contextId == None) {
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    GraphicContext* gc = malloc(sizeof(GraphicContext));
    if (gc == NULL) {
        FREE_XID(contextId);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    GC graphicContextStruct = malloc(sizeof(GC));
    if (graphicContextStruct == NULL) {
        FREE_XID(contextId);
        free(gc);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    graphicContextStruct->ext_data = NULL;
    graphicContextStruct->gid = contextId;
    SET_XID_TYPE(contextId, GRAPHICS_CONTEXT);
    SET_XID_VALUE(contextId, gc);
    gc->stipple = None;
    gc->font = None;
    gc->lineWidth = 1;
    gc->background = 0xFFFFFFFF;
    gc->foreground = 0x000000FF;
    gc->fillStyle = FillSolid;
    fprintf(stderr, "In %s, GraphicContext = %p, gid = %lu, gc = %p\n", __func__, gc, graphicContextStruct->gid, graphicContextStruct);
    if (!XChangeGC(display, graphicContextStruct, valuemask, values)) {
        XFreeGC(display, graphicContextStruct);
        return NULL;
    }
    return graphicContextStruct;
}

GContext XGContextFromGC(GC gc) {
    // https://tronche.com/gui/x/xlib/GC/XGContextFromGC.html
    return gc->gid;
}

int XChangeGC(Display* display, GC gc, unsigned long valuemask, XGCValues* values) {
    SET_X_SERVER_REQUEST(display, X_ChangeGC);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XCopyGC(Display *display, GC src, unsigned long valuemask, GC dest) {
    // https://tronche.com/gui/x/xlib/GC/XCopyGC.html
    SET_X_SERVER_REQUEST(display, X_CopyGC);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

Status XGetGCValues(Display* display, GC gc, unsigned long valuemask, XGCValues* values_return) {
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 0;
}

int XSetTSOrigin(Display* display, GC gc, int ts_x_origin, int ts_y_origin) {
    //https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetTSOrigin.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetClipOrigin(Display* display, GC gc, int clip_x_origin, int clip_y_origin) {
    // https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetClipOrigin.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetDashes(Display* display, GC gc, int dash_offset, _Xconst char dash_list[], int n) {
    // https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetDashes.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetClipMask(Display* display, GC gc, Pixmap pixmap) {
    // http://www.net.uom.gr/Books/Manuals/xlib/GC/convenience-functions/XSetClipMask.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetForeground(Display* display, GC gc, unsigned long foreground) {
    // https://linux.die.net/man/3/xsetforeground
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetFont(Display* display, GC gc, Font font) {
    // http://www.net.uom.gr/Books/Manuals/xlib/GC/convenience-functions/XSetFont.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}
