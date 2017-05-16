#include <X11/Xlib.h>
#include "gc.h"
#include "display.h"
#include "drawing.h"


int XFreeGC(Display* display, GC gc) {
    SET_X_SERVER_REQUEST(display, X_FreeGC);
    GraphicContext* gContext = GET_GC(gc);
    if (gContext->stipple != None) {
        XFreePixmap(display, gContext->stipple);
    }
    if (gContext->tile != None) {
        XFreePixmap(display, gContext->tile);
    }    
    if (gContext->clipMask != None) {
        XFreePixmap(display, gContext->clipMask);
    }
    if (gContext->dashes != NULL) {
        free(gContext->dashes);
    }
    free(gContext);
    XExtData* extData = gc->ext_data;
    while (extData != NULL) {
        XExtData* data = extData;
        extData = data->next;
        free(data);
    }
    if (gc->ext_data != NULL) {
        free(gc->ext_data);
    }
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
    GC graphicContextStruct = malloc(sizeof(struct _XGC));
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
    // Initialize default values
    gc->dashes = malloc(sizeof(char) * 2);
    if (gc->dashes == NULL) {
        XFreeGC(display, graphicContextStruct);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    gc->numDashes = 2;
    gc->dashes[0] = 4;
    gc->dashes[1] = 4;
    gc->function = GXcopy;
    gc->planeMask = 0xFFFFFFFF;
    gc->foreground = 0;
    gc->background = 1;
    gc->lineWidth = 0;
    gc->lineStyle = LineSolid;
    gc->capStyle = CapButt;
    gc->joinStyle = JoinMiter;
    gc->fillStyle = FillSolid;
    gc->fillRule = EvenOddRule;
    gc->arcMode = ArcPieSlice;
    gc->tile = None;
    gc->stipple = None;
    gc->tileStipOriginX = 0;
    gc->tileStipOriginY = 0;
    gc->font = None;
    gc->subWindowMode = ClipByChildren;
    gc->graphicsExposures = True;
    gc->clipOriginX = 0;
    gc->clipOriginY = 0;
    gc->clipMask = None;
    gc->dashOffset = 0;
    if (!XChangeGC(display, graphicContextStruct, valuemask, values)) {
        XFreeGC(display, graphicContextStruct);
        return NULL;
    }
    if (gc->tile == None) {
        gc->tile = XCreatePixmap(display, d, 2, 2, 32);
        if (gc->tile == None) {
            XFreeGC(display, graphicContextStruct);
            return NULL;
        }
        SDL_Color color;
        color.a = GET_ALPHA_FROM_COLOR(gc->foreground);
        color.r = GET_RED_FROM_COLOR(gc->foreground);
        color.g = GET_GREEN_FROM_COLOR(gc->foreground);
        color.b = GET_BLUE_FROM_COLOR(gc->foreground);
        GPU_RectangleFilled(GET_PIXMAP_IMAGE(gc->tile)->target, 0 , 0, 2, 2, color);
    }
    if (gc->stipple == None) {
        gc->stipple = XCreatePixmap(display, d, 2, 2, 1);
        if (gc->stipple == None) {
            XFreeGC(display, graphicContextStruct);
            return NULL;
        }
        SDL_Color color = {0xFF, 0xFF, 0xFF, 0xFF};
        GPU_RectangleFilled(GET_PIXMAP_IMAGE(gc->tile)->target, 0 , 0, 2, 2, color);
    }
    return graphicContextStruct;
}

GContext XGContextFromGC(GC gc) {
    // https://tronche.com/gui/x/xlib/GC/XGContextFromGC.html
    return gc->gid;
}

Bool setDashes(Display* display, GraphicContext* gc, const char dashes[], size_t numDashes, Bool verifyValues) {
    if (verifyValues) {
        size_t i;
        for (i = 0; i < numDashes; i++) {
            if (dashes[i] == 0) {
                handleError(0, display, None, 0, BadValue, 0);
                return False;
            }
        }
    }
    if (gc->numDashes != numDashes) {
        free(gc->dashes);
        gc->numDashes = 0;
        gc->dashes = malloc(sizeof(char) * numDashes);
        if (gc->dashes == NULL) {
            handleOutOfMemory(0, display, 0, 0);
            return False;
        }
        gc->numDashes = numDashes;
    }
    memcpy(gc->dashes, dashes, sizeof(char) * numDashes);
    return True;
}

int XChangeGC(Display* display, GC gc, unsigned long valuemask, XGCValues* values) {
    // https://tronche.com/gui/x/xlib/GC/XChangeGC.html
    SET_X_SERVER_REQUEST(display, X_ChangeGC);
    if (valuemask != 0 && values == NULL) {
        handleError(0, display, NULL, 0, BadValue, 0);
        return 0;
    }
    GraphicContext* graphicContext = GET_GC(gc);
    if (HAS_VALUE(valuemask, GCFunction)) {graphicContext->function = values->function;}
    if (HAS_VALUE(valuemask, GCPlaneMask)) {graphicContext->planeMask = values->plane_mask;}
    if (HAS_VALUE(valuemask, GCForeground)) {XSetForeground(display, gc, values->foreground);}
    if (HAS_VALUE(valuemask, GCBackground)) {graphicContext->background = values->background;}
    if (HAS_VALUE(valuemask, GCLineWidth)) {graphicContext->lineWidth = values->line_width;}
    if (HAS_VALUE(valuemask, GCLineStyle)) {graphicContext->lineStyle = values->line_style;}
    if (HAS_VALUE(valuemask, GCCapStyle)) {graphicContext->capStyle = values->cap_style;}
    if (HAS_VALUE(valuemask, GCJoinStyle)) {graphicContext->joinStyle = values->join_style;}
    if (HAS_VALUE(valuemask, GCFillStyle)) {graphicContext->fillStyle = values->fill_style;}
    if (HAS_VALUE(valuemask, GCFillRule)) {graphicContext->fillRule = values->fill_rule;}
    if (HAS_VALUE(valuemask, GCTile)) {
        TYPE_CHECK(values->tile, PIXMAP, display, 0);
        if (graphicContext->tile != None) {XFreePixmap(display, graphicContext->tile);}
        SET_X_SERVER_REQUEST(display, X_ChangeGC);
        graphicContext->tile = values->tile;
    }
    if (HAS_VALUE(valuemask, GCStipple)) {
        TYPE_CHECK(values->stipple, PIXMAP, display, 0);
        if (graphicContext->stipple != None) {XFreePixmap(display, graphicContext->stipple);}
        SET_X_SERVER_REQUEST(display, X_ChangeGC);
        graphicContext->stipple = values->stipple;
    }
    if (HAS_VALUE(valuemask, GCTileStipXOrigin)) {graphicContext->tileStipOriginX = values->ts_x_origin;}
    if (HAS_VALUE(valuemask, GCTileStipYOrigin)) {graphicContext->tileStipOriginY = values->ts_y_origin;}
    if (HAS_VALUE(valuemask, GCFont)) {
        if (!XSetFont(display, gc, values->font)) return 0;
    }
    if (HAS_VALUE(valuemask, GCSubwindowMode)) {graphicContext->subWindowMode = values->subwindow_mode;}
    if (HAS_VALUE(valuemask, GCGraphicsExposures)) {graphicContext->graphicsExposures = values->graphics_exposures;}
    if (HAS_VALUE(valuemask, GCClipXOrigin)) {graphicContext->clipOriginX = values->clip_x_origin;}
    if (HAS_VALUE(valuemask, GCClipYOrigin)) {graphicContext->clipOriginY = values->clip_y_origin;}
    if (HAS_VALUE(valuemask, GCClipMask)) {
        if (!XSetClipMask(display, gc, values->clip_mask)) return 0;
    }
    if (HAS_VALUE(valuemask, GCDashOffset)) {graphicContext->dashOffset = values->dash_offset;}
    if (HAS_VALUE(valuemask, GCDashList)) {
        const char value[] = {values->dashes, values->dashes};
        if (!setDashes(display, graphicContext, value, 2, true)) return 0;
    }
    if (HAS_VALUE(valuemask, GCArcMode)) {graphicContext->arcMode = values->arc_mode;}
    return 1;
}

int XCopyGC(Display *display, GC src, unsigned long valuemask, GC dest) {
    // https://tronche.com/gui/x/xlib/GC/XCopyGC.html
    SET_X_SERVER_REQUEST(display, X_CopyGC);
    XGCValues gcValues;
    if (!XGetGCValues(display, src, valuemask, &gcValues)) return 0;
    GraphicContext* srcGraphicContext = GET_GC(src);
    gcValues.clip_mask = srcGraphicContext->clipMask;
    if (!XChangeGC(display, dest, valuemask, &gcValues)) return 0;
    return setDashes(display, GET_GC(dest), srcGraphicContext->dashes, srcGraphicContext->numDashes, false) ? 1 : 0;
}

Status XGetGCValues(Display* display, GC gc, unsigned long valuemask, XGCValues* values_return) {
    // https://tronche.com/gui/x/xlib/GC/XGetGCValues.html
    GraphicContext* graphicContext = GET_GC(gc);
    if (HAS_VALUE(valuemask, GCFunction)) {values_return->function = graphicContext->function;}
    if (HAS_VALUE(valuemask, GCPlaneMask)) {values_return->plane_mask = graphicContext->planeMask;}
    if (HAS_VALUE(valuemask, GCForeground)) {values_return->foreground = graphicContext->foreground;}
    if (HAS_VALUE(valuemask, GCBackground)) {values_return->background = graphicContext->background;}
    if (HAS_VALUE(valuemask, GCLineWidth)) {values_return->line_width = graphicContext->lineWidth;}
    if (HAS_VALUE(valuemask, GCLineStyle)) {values_return->line_style = graphicContext->lineStyle;}
    if (HAS_VALUE(valuemask, GCCapStyle)) {values_return->cap_style = graphicContext->capStyle;}
    if (HAS_VALUE(valuemask, GCJoinStyle)) {values_return->join_style = graphicContext->joinStyle;}
    if (HAS_VALUE(valuemask, GCFillStyle)) {values_return->fill_style = graphicContext->fillStyle;}
    if (HAS_VALUE(valuemask, GCFillRule)) {values_return->fill_rule = graphicContext->fillRule;}
    if (HAS_VALUE(valuemask, GCTile)) {
        if (graphicContext->tile == None) {values_return->tile = 0xFFFFFFFF;}
        else {values_return->tile = graphicContext->tile;}
    }
    if (HAS_VALUE(valuemask, GCStipple)) {
        if (graphicContext->stipple != None) {values_return->stipple = 0xFFFFFFFF;}
        else {values_return->stipple = graphicContext->stipple;}
    }
    if (HAS_VALUE(valuemask, GCTileStipXOrigin)) {values_return->ts_x_origin = graphicContext->tileStipOriginX;}
    if (HAS_VALUE(valuemask, GCTileStipYOrigin)) {values_return->ts_y_origin = graphicContext->tileStipOriginY;}
    if (HAS_VALUE(valuemask, GCFont)) {
        if (graphicContext->font == None) {values_return->font = 0xFFFFFFFF;}
        else {values_return->font = graphicContext->font;}
    }
    if (HAS_VALUE(valuemask, GCSubwindowMode)) {values_return->subwindow_mode = graphicContext->subWindowMode;}
    if (HAS_VALUE(valuemask, GCGraphicsExposures)) {values_return->graphics_exposures = graphicContext->graphicsExposures;}
    if (HAS_VALUE(valuemask, GCClipXOrigin)) {values_return->clip_x_origin = graphicContext->clipOriginX;}
    if (HAS_VALUE(valuemask, GCClipYOrigin)) {values_return->clip_y_origin = graphicContext->clipOriginY;}
    if (HAS_VALUE(valuemask, GCDashOffset)) {values_return->dash_offset = graphicContext->dashOffset;}
    if (HAS_VALUE(valuemask, GCArcMode)) {values_return->arc_mode = graphicContext->arcMode;}
    return 1;
}

int XSetTSOrigin(Display* display, GC gc, int ts_x_origin, int ts_y_origin) {
    //https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetTSOrigin.html
    (void) display;
    GET_GC(gc)->tileStipOriginX = ts_x_origin;
    GET_GC(gc)->tileStipOriginY = ts_y_origin;
    return 1;
}

int XSetClipOrigin(Display* display, GC gc, int clip_x_origin, int clip_y_origin) {
    // https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetClipOrigin.html
    (void) display;
    GET_GC(gc)->clipOriginX = clip_x_origin;
    GET_GC(gc)->clipOriginY = clip_y_origin;
    return 1;
}

int XSetDashes(Display* display, GC gc, int dash_offset, _Xconst char dash_list[], int n) {
    // https://tronche.com/gui/x/xlib/GC/convenience-functions/XSetDashes.html
    if (n < 1) {
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    GraphicContext* graphicContext = GET_GC(gc);
    if (!setDashes(display, graphicContext, dash_list, (size_t) n, true)) return 0;
    graphicContext->dashOffset = dash_offset;
    return 1;
}

int XSetClipMask(Display* display, GC gc, Pixmap pixmap) {
    // http://www.net.uom.gr/Books/Manuals/xlib/GC/convenience-functions/XSetClipMask.html
    TYPE_CHECK(pixmap, PIXMAP, display, 0);
    GraphicContext* graphicContext = GET_GC(gc);
    if (graphicContext->clipMask != None) {XFreePixmap(display, graphicContext->clipMask);}
    SET_X_SERVER_REQUEST(display, X_ChangeGC);
    graphicContext->clipMask = pixmap;
    return 1;
}

int XSetForeground(Display* display, GC gc, unsigned long foreground) {
    // https://linux.die.net/man/3/xsetforeground
    (void) display;
    GET_GC(gc)->foreground = foreground;
    return 1;
}

int XSetFont(Display* display, GC gc, Font font) {
    // http://www.net.uom.gr/Books/Manuals/xlib/GC/convenience-functions/XSetFont.html
    TYPE_CHECK(font, FONT, display, 0);
    GET_GC(gc)->font = font;
    return 1;
}
