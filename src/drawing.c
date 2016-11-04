#include <X11/Xlib.h>
#include "drawing.h"
#include "errors.h"
#include "window.h"
#include "SDL_gpu.h"
#include "display.h"

/*
 * Flip all screen children and cause them to draw their content to the screen.
 */
void flipScreen() {
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    size_t i;
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->renderTarget != NULL) {
            GPU_Target* target = GET_WINDOW_STRUCT(children[i])->renderTarget;
//            SDL_Color color = { 0x00, 0xFF, 0x00, 0xFF };
//            int w, h;
//            GET_WINDOW_DIMS(children[i], w, h);
//            GPU_Rect rect;
//            rect.x = 0; rect.y = h - 20;
//            rect.w = 20; rect.h = 20;
//            GPU_RectangleFilled2(target, rect, color);
            GPU_Flip(target);
        }
    }
//    printWindowsHierarchy();
#ifdef DEBUG_WINDOWS
//    drawWindowsDebugSurfacePlane();
//    drawWindowsDebugBorder();
#endif
}

/*
 * Get a render target for this window. If this window is unmapped, a render target to
 * its own unmappedContent image is returned. If the window is a mapped top level window,
 * then the target to the window is returned. If None of the above applies to the given
 * window, a parent of the window is searched that meets the requirements. The render
 * target of that parent is then returned, but with the correct viewport of the original
 * window.
 */
GPU_Target* getWindowRenderTarget(Window window) {
    Window targetWindow = window;
    int x = 0, y = 0, w = 0, h = 0;
    GPU_Rect viewPort = {0, 0, 0, 0};
    GET_WINDOW_DIMS(window, viewPort.w, viewPort.h);
//    if (viewPort.w <= 1 && viewPort.h <= 1) abort();
    while (GET_PARENT(targetWindow) != NULL && GET_WINDOW_STRUCT(targetWindow)->sdlWindow == NULL
           && GET_WINDOW_STRUCT(targetWindow)->mapState != UnMapped) {
        GET_WINDOW_DIMS(targetWindow, w, h);
        if (viewPort.w > w - viewPort.x) viewPort.w = w - viewPort.x;
        if (viewPort.h > h - viewPort.y) viewPort.h = h - viewPort.y;
        GET_WINDOW_POS(targetWindow, x, y);
        viewPort.x += x;
        viewPort.y += y;
        targetWindow = GET_PARENT(targetWindow);
    }
    if (targetWindow == SCREEN_WINDOW) {
        fprintf(stderr, "Failed to find a render target in %s for window %p!\n", __func__, window);
#ifdef DEBUG_WINDOWS
        printWindowsHierarchy();
#endif
        return NULL;
    }
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(targetWindow);
    if (windowStruct->mapState == UnMapped) {
        if (windowStruct->unmappedContent == NULL) {
            windowStruct->unmappedContent = GPU_CreateImage((Uint16) windowStruct->w,
                                                            (Uint16) windowStruct->h,
                                                            GPU_FORMAT_RGBA);
            if (windowStruct->unmappedContent == NULL) {
                fprintf(stderr, "GPU_CreateImage failed in %s for window %p: %s\n", __func__,
                        window, GPU_PopErrorCode().details);
                return NULL;
            }
        }
        if (windowStruct->renderTarget == NULL) {
            windowStruct->renderTarget = GPU_LoadTarget(windowStruct->unmappedContent);
            if (windowStruct->renderTarget == NULL) {
                fprintf(stderr, "GPU_LoadTarget failed in %s for window %p: %s\n", __func__,
                        window, GPU_PopErrorCode().details);
                return NULL;
            }
        }
    } else if (IS_MAPPED_TOP_LEVEL_WINDOW(targetWindow)) {
        if (windowStruct->renderTarget == NULL) {
            fprintf(stderr, "Got window (%p) with sdl window (%d) but no target in %s\n",
                    window, SDL_GetWindowID(windowStruct->sdlWindow), __func__);
            return NULL;
        }
    } else {
        fprintf(stderr, "Failed to find a render target in %s for window %p!\n", __func__, window);
        return NULL;
    }
    GPU_SetViewport(windowStruct->renderTarget, viewPort);
    GPU_SetClipRect(windowStruct->renderTarget, viewPort);
    fprintf(stderr, "Render viewport is {x = %f, y = %f, w = %f, h = %f}\n", viewPort.x, viewPort.y, viewPort.w, viewPort.h);
    return windowStruct->renderTarget;
}

void XFillPolygon(Display* display, Drawable d, GC gc, XPoint *points, int npoints, int shape, int mode) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillPolygon.html
    SET_X_SERVER_REQUEST(display, XCB_FILL_POLY);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XFillArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillArc.html
    SET_X_SERVER_REQUEST(display, XCB_POLY_FILL_ARC);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XDrawArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawArc.html
    SET_X_SERVER_REQUEST(display, XCB_POLY_ARC);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XCopyPlane(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y, unsigned long plane) {
    // https://tronche.com/gui/x/xlib/graphics/XCopyPlane.html
    SET_X_SERVER_REQUEST(display, XCB_COPY_PLANE);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XDrawLines(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int mode) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawLines.html
    SET_X_SERVER_REQUEST(display, XCB_POLY_LINE);
    TYPE_CHECK(d, DRAWABLE, display);
    fprintf(stderr, "%s: Drawing on %p\n", __func__, d);
    if (npoints <= 1) {
        fprintf(stderr, "Invalid number of points in %s: %d\n", __func__, npoints);
        handleError(0, display, NULL, 0, BadValue, 0);
        return;
    }
    if (mode != CoordModeOrigin && mode != CoordModePrevious) {
        fprintf(stderr, "Bad mode give to %s: %d\n", __func__, mode);
        handleError(0, display, NULL, 0, BadValue, 0);
        return;
    }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %p in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return;
    }
    fprintf(stderr, "%s: Drawing on render target %p\n", __func__, renderTarget);
    GPU_SetLineThickness(gc->line_width);
    SDL_Color drawColor = {
            GET_RED_FROM_COLOR(gc->foreground),
            GET_GREEN_FROM_COLOR(gc->foreground),
            GET_BLUE_FROM_COLOR(gc->foreground),
            GET_ALPHA_FROM_COLOR(gc->foreground),
    };
    size_t i;
    XPoint last, current;
    last = points[0];
    for (i = 1; i < npoints; i++) {
        current = points[i];
        if (mode == CoordModePrevious) {
            current.x += last.x;
            current.y += last.y;
        }
        fprintf(stderr, "Drawing line {x1 = %d, y1 = %d, x2 = %d, y2 = %d}\n", last.x, last.y, current.x, current.y);
        GPU_Line(renderTarget, last.x, last.y, current.x, current.y, drawColor);
        last = current;
    }
/**/    GPU_Flip(renderTarget);
}

void XCopyArea(Display* display, Drawable src, Drawable dest, GC gc, int src_x, int src_y,
               unsigned int width, unsigned int height, int dest_x, int dest_y) {
    // https://tronche.com/gui/x/xlib/graphics/XCopyArea.html
    SET_X_SERVER_REQUEST(display, XCB_COPY_AREA);
    TYPE_CHECK(src, DRAWABLE, display);
    TYPE_CHECK(dest, DRAWABLE, display);
    fprintf(stderr, "%s: Copy area from %p to %p\n", __func__, src, dest);
    if (IS_TYPE(src, WINDOW)) {
        if (IS_INPUT_ONLY(src)) {
            fprintf(stderr, "BadMatch: Got input only window as the source in %s!\n", __func__);
            handleError(0, display, src, 0, BadMatch, 0);
            return;
        } else if (GET_WINDOW_STRUCT(src)->mapState == UnMapped && GET_WINDOW_STRUCT(src)->unmappedContent == NULL) {
            return;
        }
    }
    if (IS_TYPE(dest, WINDOW) && IS_INPUT_ONLY(dest)) {
        fprintf(stderr, "BadMatch: Got input only window as the destination in %s!\n", __func__);
        handleError(0, display, dest, 0, BadMatch, 0);
        return;
    }
    GPU_Target* sourceTarget;
    GET_RENDER_TARGET(src, sourceTarget);
    if (sourceTarget == NULL) {
        fprintf(stderr, "BadMatch: Failed to get render target of source drawable %p in %s!\n",
                src, __func__);
        handleError(0, display, src, 0, BadMatch, 0);
        return;
    }
    GPU_Image* sourceImage = GPU_CopyImageFromTarget(sourceTarget);
    if (sourceImage == NULL) {
        fprintf(stderr, "BadMatch: Failed to get the source image from the source target of the "
                "source drawable %p in %s!\n", src, __func__);
        handleError(0, display, src, 0, BadMatch, 0);
        return;
    }
    GPU_Target* renderDest;
    GET_RENDER_TARGET(dest, renderDest);
    if (renderDest == NULL) {
        fprintf(stderr, "BadMatch: Failed to get render target of destination drawable %p in %s!\n",
                dest, __func__);
        handleError(0, display, dest, 0, BadMatch, 0);
        return;
    }
    fprintf(stderr, "%s: Copy area from target %p to target %p\n", __func__, sourceTarget, renderDest);
    GPU_Rect sourceRect = {
            src_x + 1,
            src_y + 1,
            width - 2,
            height -2
    };

    fprintf(stderr, "Copy area {x = %f, y = %f, w = %f, h = %f}\n", sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h);
    GPU_Blit(sourceImage, &sourceRect, renderDest, dest_x + sourceRect.w / 2, dest_y + sourceRect.h / 2);
    GPU_FreeImage(sourceImage);
/**/    GPU_Flip(renderDest);
    
    // TODO: Events
}

void XDrawRectangle(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, int height) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawRectangle.html
    SET_X_SERVER_REQUEST(display, XCB_POLY_RECTANGLE);
    TYPE_CHECK(d, DRAWABLE, display);
    fprintf(stderr, "%s: Drawing on %p\n", __func__, d);
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %p in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return;
    }
    GPU_SetLineThickness(gc->line_width);
    SDL_Color drawColor = {
            GET_RED_FROM_COLOR(gc->foreground),
            GET_GREEN_FROM_COLOR(gc->foreground),
            GET_BLUE_FROM_COLOR(gc->foreground),
            GET_ALPHA_FROM_COLOR(gc->foreground),
    };
    fprintf(stderr, "{x = %d, y = %d, w = %d, h = %d}\n", x, y, width, height);
    GPU_Rect rectangle = {x, y, width, height};
    fprintf(stderr, "Drawing rectangle {x = %f, y = %f, w = %f, h = %f}\n", rectangle.x, rectangle.y, rectangle.w, rectangle.h);
    GPU_Rectangle2(renderTarget, rectangle, drawColor);
/**/    GPU_Flip(renderTarget);
}

void XFillRectangles(Display *display, Drawable d, GC gc, XRectangle *rectangles, int nrectangles) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillRectangles.html
    SET_X_SERVER_REQUEST(display, XCB_POLY_FILL_RECTANGLE);
    TYPE_CHECK(d, DRAWABLE, display);
    fprintf(stderr, "%s: Drawing on %p\n", __func__, d);
    if (nrectangles < 1) {
        fprintf(stderr, "Invalid number of rectangles in %s: %d\n", __func__, nrectangles);
        handleError(0, display, NULL, 0, BadValue, 0);
        return;
    }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %p in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return;
    }
    if (renderTarget->context != NULL) {
        fprintf(stderr, "%s: Render target: %p, render target context = %p\n", __func__,
                renderTarget, (SDL_GLContext) renderTarget->context->context);
    }
    GPU_SetLineThickness(gc->line_width);
    fprintf(stderr, "bgColor: 0x%08lx, fgColor: 0x%08lx\n", gc->background, gc->foreground);
    if (gc->fill_style == FillSolid) {
        size_t i;
        SDL_Color drawColor = {
                GET_RED_FROM_COLOR(gc->foreground),
                GET_GREEN_FROM_COLOR(gc->foreground),
                GET_BLUE_FROM_COLOR(gc->foreground),
                GET_ALPHA_FROM_COLOR(gc->foreground),
        };
        fprintf(stderr, "%s: Color {r = %d, g = %d, b = %d, a = %d}\n", __func__, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
        fprintf(stderr, "Render viewport is {x = %f, y = %f, w = %f, h = %f}\n", renderTarget->viewport.x, renderTarget->viewport.y, renderTarget->viewport.w, renderTarget->viewport.h);
        for (i = 0; i < nrectangles; i++) {
            GPU_Rect rectangle = {
                rectangles[i].x,
                rectangles[i].y,
                rectangles[i].width,
                rectangles[i].height,
            };
            fprintf(stderr, "Drawing filled rectangle {x = %f, y = %f, w = %f, h = %f}\n", rectangle.x, rectangle.y, rectangle.w, rectangle.h);
            GPU_RectangleFilled2(renderTarget, rectangle, drawColor);
        }
    } else if (gc->fill_style == FillTiled) {
        fprintf(stderr, "Fill_style is %s\n", "FillTiled");
    } else if (gc->fill_style == FillOpaqueStippled) {
        fprintf(stderr, "Fill_style is %s\n", "FillOpaqueStippled");
    } else if (gc->fill_style == FillStippled) {
        fprintf(stderr, "Fill_style is %s\n", "FillStippled");
    }
/**/    GPU_Flip(renderTarget);
}
