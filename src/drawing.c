#include <X11/Xlib.h>
#include "drawing.h"
#include "errors.h"
#include "window.h"
#include "SDL_gpu.h"
#include "display.h"
#include "util.h"
#include "gc.h"

/*
 * Flip all screen children and cause them to draw their content to the screen.
 */
void flipScreen() {
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    size_t i;
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->children.length; i++) {
        if (GET_WINDOW_STRUCT(children[i])->renderTarget != NULL) {
            GPU_Target* target = GET_WINDOW_STRUCT(children[i])->renderTarget;
            GPU_Flip(target);
        }
    }
#ifdef DEBUG_WINDOWS
//    printWindowsHierarchy();
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
    GPU_Rect clipRect = {0, 0, 0, 0};
    GET_WINDOW_DIMS(window, clipRect.w, clipRect.h);
    while (GET_PARENT(targetWindow) != None && GET_WINDOW_STRUCT(targetWindow)->sdlWindow == NULL
           && GET_WINDOW_STRUCT(targetWindow)->mapState != UnMapped) {
        GET_WINDOW_DIMS(targetWindow, w, h);
        if (clipRect.w > w - clipRect.x) clipRect.w = w - clipRect.x;
        if (clipRect.h > h - clipRect.y) clipRect.h = h - clipRect.y;
        GET_WINDOW_POS(targetWindow, x, y);
        clipRect.x += x;
        clipRect.y += y;
        targetWindow = GET_PARENT(targetWindow);
    }
    if (targetWindow == SCREEN_WINDOW) {
        fprintf(stderr, "Failed to find a render target in %s for window %lu!\n", __func__, window);
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
                fprintf(stderr, "GPU_CreateImage failed in %s for window %lu: %s\n", __func__,
                        window, GPU_PopErrorCode().details);
                return NULL;
            }
        }
        if (windowStruct->renderTarget == NULL) {
            windowStruct->renderTarget = GPU_LoadTarget(windowStruct->unmappedContent);
            if (windowStruct->renderTarget == NULL) {
                fprintf(stderr, "GPU_LoadTarget failed in %s for window %lu: %s\n", __func__,
                        window, GPU_PopErrorCode().details);
                return NULL;
            }
        }
    } else if (IS_MAPPED_TOP_LEVEL_WINDOW(targetWindow)) {
        if (windowStruct->renderTarget == NULL) {
            fprintf(stderr, "Got window (%lu) with sdl window (%d) but no target in %s\n",
                    window, SDL_GetWindowID(windowStruct->sdlWindow), __func__);
            return NULL;
        }
        if (GPU_GetContextTarget() == NULL) {
            GPU_MakeCurrent(windowStruct->renderTarget, SDL_GetWindowID(windowStruct->sdlWindow));
        }
    } else {
        fprintf(stderr, "Failed to find a render target in %s for window %lu!\n", __func__, window);
        return NULL;
    }
    GPU_SetClipRect(windowStruct->renderTarget, clipRect);
    GPU_Rect viewPort;
    viewPort.x = clipRect.x;
    viewPort.y = clipRect.y;
    GET_WINDOW_DIMS(SCREEN_WINDOW, viewPort.w, viewPort.h);
    GPU_SetViewport(windowStruct->renderTarget, viewPort);
    fprintf(stderr, "Render viewport is {x = %d, y = %d, w = %d, h = %d}\n", (int) viewPort.x, (int) viewPort.y, (int) viewPort.w, (int) viewPort.h);
    return windowStruct->renderTarget;
}

int XFillPolygon(Display* display, Drawable d, GC gc, XPoint *points, int npoints, int shape, int mode) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillPolygon.html
    SET_X_SERVER_REQUEST(display, X_FillPoly);
    TYPE_CHECK(d, DRAWABLE, display, 0);
    if (npoints <= 1) {
        fprintf(stderr, "Invalid number of points in %s: %d\n", __func__, npoints);
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %lu in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return 0;
    }
    float* fPoints = malloc(sizeof(float) * npoints * 2);
    if (fPoints == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        return 0;
    }
    GraphicContext* gContext = GET_GC(gc);
    GPU_SetLineThickness(gContext->lineWidth);
    SDL_Color drawColor = {
            GET_RED_FROM_COLOR(gContext->foreground),
            GET_GREEN_FROM_COLOR(gContext->foreground),
            GET_BLUE_FROM_COLOR(gContext->foreground),
            GET_ALPHA_FROM_COLOR(gContext->foreground),
    };
    size_t i;
    fPoints[0] = points[0].x;
    fPoints[1] = points[0].y;
    for (i = 2; i < npoints * 2; i += 2) {
        if (mode == CoordModePrevious) {
            fPoints[i]     = fPoints[i - 2] + points[i].x;
            fPoints[i + 1] = fPoints[i - 1] + points[i].y;
        } else {
            fPoints[i]     = points[i].x;
            fPoints[i + 1] = points[i].y;
        }
    }
    GPU_PolygonFilled(renderTarget, (unsigned int) npoints, fPoints, drawColor);
    free(fPoints);
    return 1;
}

int XFillArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillArc.html
    SET_X_SERVER_REQUEST(display, X_PolyFillArc);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XDrawArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawArc.html
    SET_X_SERVER_REQUEST(display, X_PolyArc);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XCopyPlane(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y, unsigned long plane) {
    // https://tronche.com/gui/x/xlib/graphics/XCopyPlane.html
    SET_X_SERVER_REQUEST(display, X_CopyPlane);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XDrawLine(Display* display, Drawable d, GC gc, int x1, int y1, int x2, int y2) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawLine.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XDrawLines(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int mode) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawLines.html
    SET_X_SERVER_REQUEST(display, X_PolyLine);
    TYPE_CHECK(d, DRAWABLE, display, 0);
    fprintf(stderr, "%s: Drawing on %lu\n", __func__, d);
    if (npoints <= 1) {
        fprintf(stderr, "Invalid number of points in %s: %d\n", __func__, npoints);
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    if (mode != CoordModeOrigin && mode != CoordModePrevious) {
        fprintf(stderr, "Bad mode give to %s: %d\n", __func__, mode);
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %lu in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return 0;
    }
    fprintf(stderr, "%s: Drawing on render target %p\n", __func__, renderTarget);
    GraphicContext* gContext = GET_GC(gc);
    GPU_SetLineThickness(gContext->lineWidth);
    SDL_Color drawColor = {
            GET_RED_FROM_COLOR(gContext->foreground),
            GET_GREEN_FROM_COLOR(gContext->foreground),
            GET_BLUE_FROM_COLOR(gContext->foreground),
            GET_ALPHA_FROM_COLOR(gContext->foreground),
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
    GPU_Flip(renderTarget);
    return 1;
}

int XCopyArea(Display* display, Drawable src, Drawable dest, GC gc, int src_x, int src_y,
               unsigned int width, unsigned int height, int dest_x, int dest_y) {
    // https://tronche.com/gui/x/xlib/graphics/XCopyArea.html
    SET_X_SERVER_REQUEST(display, X_CopyArea);
    TYPE_CHECK(src, DRAWABLE, display, 0);
    TYPE_CHECK(dest, DRAWABLE, display, 0);
    fprintf(stderr, "%s: Copy area from %lu to %lu\n", __func__, src, dest);
    if (IS_TYPE(src, WINDOW)) {
        if (IS_INPUT_ONLY(src)) {
            fprintf(stderr, "BadMatch: Got input only window as the source in %s!\n", __func__);
            handleError(0, display, src, 0, BadMatch, 0);
            return 0;
        } else if (GET_WINDOW_STRUCT(src)->mapState == UnMapped && GET_WINDOW_STRUCT(src)->unmappedContent == NULL) {
            return 0;
        }
    }
    if (IS_TYPE(dest, WINDOW) && IS_INPUT_ONLY(dest)) {
        fprintf(stderr, "BadMatch: Got input only window as the destination in %s!\n", __func__);
        handleError(0, display, dest, 0, BadMatch, 0);
        return 0;
    }
    GPU_Target* sourceTarget;
    GET_RENDER_TARGET(src, sourceTarget);
    if (sourceTarget == NULL) {
        fprintf(stderr, "BadMatch: Failed to get render target of source drawable %lu in %s!\n",
                src, __func__);
        handleError(0, display, src, 0, BadMatch, 0);
        return 0;
    }
    GPU_Image* sourceImage = GPU_CopyImageFromTarget(sourceTarget);
    if (sourceImage == NULL) {
        fprintf(stderr, "BadMatch: Failed to get the source image from the source target of the "
                "source drawable %lu in %s!\n", src, __func__);
        handleError(0, display, src, 0, BadMatch, 0);
        return 0;
    }
    GPU_Target* renderDest;
    GET_RENDER_TARGET(dest, renderDest);
    if (renderDest == NULL) {
        fprintf(stderr, "BadMatch: Failed to get render target of destination drawable %lu in %s!\n",
                dest, __func__);
        handleError(0, display, dest, 0, BadMatch, 0);
        return 0;
    }
    fprintf(stderr, "%s: Copy area from target %p to target %p\n", __func__, sourceTarget, renderDest);
    GPU_Rect sourceRect = { src_x, src_y, width, height };

    fprintf(stderr, "Copy area {x = %f, y = %f, w = %f, h = %f}\n", sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h);
    GPU_Blit(sourceImage, &sourceRect, renderDest, dest_x + sourceRect.w / 2, dest_y + sourceRect.h / 2);
    GPU_FreeImage(sourceImage);
    GPU_Flip(renderDest);
    
    // TODO: Events
    return 1;
}

int XDrawRectangle(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/graphics/drawing/XDrawRectangle.html
    SET_X_SERVER_REQUEST(display, X_PolyRectangle);
    TYPE_CHECK(d, DRAWABLE, display, 0);
    fprintf(stderr, "%s: Drawing on %lu\n", __func__, d);
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %lu in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return 0;
    }
    GraphicContext* gContext = GET_GC(gc);
    GPU_SetLineThickness(gContext->lineWidth);
    SDL_Color drawColor = {
            GET_RED_FROM_COLOR(gContext->foreground),
            GET_GREEN_FROM_COLOR(gContext->foreground),
            GET_BLUE_FROM_COLOR(gContext->foreground),
            GET_ALPHA_FROM_COLOR(gContext->foreground),
    };
    fprintf(stderr, "{x = %d, y = %d, w = %d, h = %d}\n", x, y, width, height);
    GPU_Rect rectangle = {x, y, width, height};
    fprintf(stderr, "Drawing rectangle {x = %f, y = %f, w = %f, h = %f}\n", rectangle.x, rectangle.y, rectangle.w, rectangle.h);
    GPU_Rectangle2(renderTarget, rectangle, drawColor);
    GPU_Flip(renderTarget);
    return 1;
}

int XFillRectangle(Display* display, Drawable d, GC gc, int x, int y,
                   unsigned int width, unsigned int height) {
    XRectangle rectangle; // TODO: Make this not depend on XFillRectangles
    rectangle.x = x;
    rectangle.y = y;
    rectangle.width = width;
    rectangle.height = height;
    return XFillRectangles(display, d, gc, &rectangle, 1);
}

int XFillRectangles(Display *display, Drawable d, GC gc, XRectangle *rectangles, int nrectangles) {
    // https://tronche.com/gui/x/xlib/graphics/filling-areas/XFillRectangles.html
    SET_X_SERVER_REQUEST(display, X_PolyFillRectangle);
    TYPE_CHECK(d, DRAWABLE, display, 0);
    fprintf(stderr, "%s: Drawing on %lu\n", __func__, d);
    if (nrectangles < 1) {
        fprintf(stderr, "Invalid number of rectangles in %s: %d\n", __func__, nrectangles);
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(d, renderTarget);
    if (renderTarget == NULL) {
        fprintf(stderr, "Failed to get the render target of %lu in %s\n", d, __func__);
        handleError(0, display, d, 0, BadDrawable, 0);
        return 0;
    }
    if (renderTarget->context != NULL) {
        fprintf(stderr, "%s: Render target: %p, render target context = %p\n", __func__,
                renderTarget, (SDL_GLContext) renderTarget->context->context);
    }
    GraphicContext* gContext = GET_GC(gc);
    GPU_SetLineThickness(gContext->lineWidth);
    fprintf(stderr, "bgColor: 0x%08lx, fgColor: 0x%08lx\n", gContext->background, gContext->foreground);
    if (gContext->fillStyle == FillSolid) {
        size_t i;
        SDL_Color drawColor = {
                GET_RED_FROM_COLOR(gContext->foreground),
                GET_GREEN_FROM_COLOR(gContext->foreground),
                GET_BLUE_FROM_COLOR(gContext->foreground),
                GET_ALPHA_FROM_COLOR(gContext->foreground),
        };
        fprintf(stderr, "%s: Color {r = %d, g = %d, b = %d, a = %d}\n", __func__, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
        fprintf(stderr, "Render viewport is {x = %f, y = %f, w = %f, h = %f}\n", renderTarget->viewport.x, renderTarget->viewport.y, renderTarget->viewport.w, renderTarget->viewport.h);
        for (i = 0; i < nrectangles; i++) {
            GPU_Rect rectangle = {
                rectangles[i].x,
                rectangles[i].y,
                rectangles[i].width,
                rectangles[i].height
            };
            fprintf(stderr, "Drawing filled rectangle {x = %f, y = %f, w = %f, h = %f}\n", rectangle.x, rectangle.y, rectangle.w, rectangle.h);
            GPU_RectangleFilled2(renderTarget, rectangle, drawColor);
        }
    } else if (gContext->fillStyle == FillTiled) {
        fprintf(stderr, "Fill_style is %s\n", "FillTiled");
    } else if (gContext->fillStyle == FillOpaqueStippled) {
        fprintf(stderr, "Fill_style is %s\n", "FillOpaqueStippled");
        GPU_Image* stipple = GET_PIXMAP_IMAGE(gContext->stipple);
        GPU_Image* tile = GPU_CopyImage(stipple);
        if (tile == NULL) {
            fprintf(stderr, "Failed to copy image from gc->stipple: %s\n", GPU_PopErrorCode().details);
            handleError(0, display, gContext->stipple, 0 , BadMatch, 0);
            return 0;
        }
        GPU_Target* tileTarget = GPU_LoadTarget(tile);
        if (tileTarget == NULL) {
            fprintf(stderr, "Failed to create image target from gc->stipple: %s\n", GPU_PopErrorCode().details);
            handleError(0, display, gContext->stipple, 0 , BadMatch, 0);
            return 0;
        }
        GPU_SetShapeBlendFunction(GPU_FUNC_SRC_COLOR, GPU_FUNC_ZERO, GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA);
        SDL_Color color = GPU_MakeColor(GET_RED_FROM_COLOR(gContext->foreground), GET_GREEN_FROM_COLOR(gContext->foreground), GET_BLUE_FROM_COLOR(gContext->foreground), GET_ALPHA_FROM_COLOR(gContext->foreground));
        GPU_RectangleFilled(tileTarget, 0, 0, tile->w, tile->h, color);
        GPU_SetShapeBlendFunction(GPU_FUNC_SRC_COLOR, GPU_FUNC_ZERO, GPU_FUNC_ZERO, GPU_FUNC_ONE_MINUS_DST_ALPHA);
        color = GPU_MakeColor(GET_RED_FROM_COLOR(gContext->background), GET_GREEN_FROM_COLOR(gContext->background), GET_BLUE_FROM_COLOR(gContext->background), GET_ALPHA_FROM_COLOR(gContext->background));
        GPU_RectangleFilled(tileTarget, 0, 0, tile->w, tile->h, color);
        GPU_SetShapeBlendMode(GPU_BLEND_NORMAL);
        GPU_SetWrapMode(tile, GPU_WRAP_REPEAT, GPU_WRAP_REPEAT);
        
        size_t i;
        for (i = 0; i < nrectangles; i++) {
            GPU_Rect rectangle = { 0, 0, rectangles[i].width, rectangles[i].height };
            fprintf(stderr, "Drawing filled rectangle using FillOpaqueStippled {x = %f, y = %f, w = %f, h = %f}\n", rectangle.x, rectangle.y, rectangle.w, rectangle.h);
            GPU_Blit(tile, &rectangle, renderTarget, rectangles[i].x, rectangles[i].y);
        }
        GPU_FreeImage(tile);
    } else if (gContext->fillStyle == FillStippled) {
        fprintf(stderr, "Fill_style is %s\n", "FillStippled");
    }
    GPU_Flip(renderTarget);
    return 1;
}
