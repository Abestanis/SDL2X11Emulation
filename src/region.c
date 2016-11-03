#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <inttypes.h>
#include "pixman.h"
#include "drawing.h"
#include "resourceTypes.h"

// TODO: check boolean returns
void XDestroyRegion(Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XDestroyRegion.html
    pixman_region_fini(region);
//    int i;
//    for (i = 0; i < region->numRects; i++) {
//        free(&region->rects[i]);
//    }
//    free(region);
}

Region XCreateRegion() {
    // https://tronche.com/gui/x/xlib/utilities/regions/XCreateRegion.html
    Region region = malloc(sizeof(Region));
    if (region == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate Region structure in XCreateRegion!\n");
        return NULL;
    }
    pixman_region_init(region);
//    region->size = 0;
//    region->numRects = 0;
//    region->rects = NULL;
//    // Outside Box of region
//    region->extents.x1 = 0;
//    region->extents.x2 = 0;
//    region->extents.y1 = 0;
//    region->extents.y2 = 0;
    return region;
}

Bool XEmptyRegion(Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XEmptyRegion.html
    return !pixman_region_not_empty(region);
//    return !REGION_NOT_EMPTY(region);
}

int XRectInRegion(Region region, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XRectInRegion.html
    pixman_region_overlap_t res;
    pixman_box16_t box;
    box.x1 = x;
    box.y1 = y;
    box.x2 = x + width;
    box.y2 = y + height;
    res = pixman_region_contains_rectangle(region, &box);
    if (res == PIXMAN_REGION_OUT) {
        return RectangleOut;
    } else if (res == PIXMAN_REGION_IN) {
        return RectangleIn;
    } else {
        return RectanglePart;
    }
//    Box box;
//    box.x1 = x;
//    box.x2 = x + width;
//    box.y1 = y;
//    box.y2 = y + height;
//    if ((region->extents.x1 > box.x2 || region->extends.x2 < box.x1) &&
//            (region->extends.y1 > box.y2 || region->extends.y2 < box.y1)) {
//        return RectangleOut;
//    }
//    int result = RectangleOut;
//    int i;
//    for (i = 0; i < region->numRects; i++) {
//        Box rect = region->rects[i];
//        Bool boxRightToRectX1 = box.x1 >= rect.x1;
//        Bool boxLeftToRectX2  = box.x2 <= rect.x2;
//        Bool boxDownToRectY1  = box.y1 >= rect.y1;
//        Bool boxUpToRectY2    = box.y2 <= rect.y2;
//        if (boxRightToRectX1 && boxLeftToRectX2 && boxDownToRectY1 && boxUpToRectY2) {
//            return RectangleIn;
//        }
//        Bool intersectX = (!boxRightToRectX1 && !boxLeftToRectX2)
//                          || (boxRightToRectX1 && box.x1 <= rect.x2)
//                          || (boxLeftToRectX2 && box.x2 >= rect.x1);
//        Bool intersectY = (!boxDownToRectY1 && !boxUpToRectY2)
//                          || (boxDownToRectY1 && box.y1 <= rect.y2)
//                          || (boxUpToRectY2 && box.y2 >= rect.y1);
//        if (intersectX && intersectY) {
//            result = RectanglePart;
//        }
//        // FIXME: Does not detect when box is fully enclosed in multiple rectangles.
//    }
//    return result;
}

void XClipBox(Region region, XRectangle* rect_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XClipBox.html
    pixman_box16_t* extends = pixman_region_extents(region);
    rect_return->width  = abs(extends->x2 - extends->x1);
    rect_return->y      = extends->y1;
    rect_return->height = abs(extends->y2 - extends->y1);
    rect_return->x      = extends->x1;
//    rect_return->width  = abs(region->extents.x2 - region->extents.x1);
//    rect_return->y      = region->extents.y1;
//    rect_return->height = abs(region->extents.y2 - region->extents.y1);

}

void XIntersectRegion(Region sra, Region srb, Region dr_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XIntersectRegion.html
    if (!pixman_region_intersect(dr_return, sra, srb)) {
        fprintf(stderr, "pixman_region_intersect did not return true!\n");
    }
}

void XSubtractRegion(Region sra, Region srb, Region dr_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XSubtractRegion.html
    if (!pixman_region_subtract(dr_return, sra, srb)) {
        fprintf(stderr, "pixman_region_subtract did not return true!\n");
    }
}

void XUnionRectWithRegion(XRectangle *rectangle, Region src_region, Region dest_region_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XUnionRectWithRegion.html
    if (!pixman_region_union_rect(dest_region_return, src_region, rectangle->x, rectangle->y,
                                  rectangle->width, rectangle->height)) {
        fprintf(stderr, "pixman_region_union_rect did not return true!\n");
    }
}

void XSetRegion(Display* display, GC gc, Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XSetRegion.html
//    SET_X_SERVER_REQUEST(display, XCB_);
    pixman_box16_t* extends = pixman_region_extents(region);
    int width  = abs(extends->x2 - extends->x1);
    int height = abs(extends->y2 - extends->y1);
    // TODO: Implement
//    SDL_Surface* surface = NULL;
//    if (IS_TYPE(gc->clip_mask, PIXMAP)) {
//        surface = GET_SURFACE(gc->clip_mask);
//    }
//    if (surface == NULL || surface->w < width || surface->h < height) {
//        if (surface != None) {
//            SDL_FreeSurface(surface);
//        }
//        if (gc->clip_mask == None) {
//            gc->clip_mask = malloc(sizeof(XID));
//            if (gc->clip_mask == NULL) {
//                fprintf(stderr, "Out of memory: Could not allocate XID struct in XSetRegion!\n");
//                return;
//            }
//            gc->clip_mask->type = PIXMAP;
//        }
//        surface = SDL_CreateRGBSurface(0, width, height, SDL_SURFACE_DEPTH, DEFAULT_RED_MASK,
//                                        DEFAULT_GREEN_MASK, DEFAULT_BLUE_MASK, DEFAULT_ALPHA_MASK);
//        if (surface == NULL) {
//            fprintf(stderr, "CreateRGBSurface failed in XSetRegion: %s\n", SDL_GetError());
//            free(gc->clip_mask);
//            gc->clip_mask = None;
//            return;
//        }
//        gc->clip_mask->dataPointer = surface;
//    }
//    SDL_FillRect(surface, NULL, 0x111111);
//    LOCK_SURFACE(surface);
//    int x, y;
//    for (x = 0; x < width; x++) {
//        for (y = 0; y < height; y++) {
//            if (pixman_region_contains_point(region, x, y, NULL)) {
//                putPixel(surface, x, y, 0xFFFFFFFF);
//            }
//        }
//    }
//    UNLOCK_SURFACE(surface);
}
