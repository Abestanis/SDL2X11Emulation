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
}

Region XCreateRegion() {
    // https://tronche.com/gui/x/xlib/utilities/regions/XCreateRegion.html
    Region region = malloc(sizeof(Region));
    if (region == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate Region structure in XCreateRegion!\n");
        return NULL;
    }
    pixman_region_init(region);
    return region;
}

Bool XEmptyRegion(Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XEmptyRegion.html
    return !pixman_region_not_empty(region);
}

int XRectInRegion(Region region, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XRectInRegion.html
    pixman_region_overlap_t res;
    pixman_box16_t box;
    box.x1 = (int16_t) x;
    box.y1 = (int16_t) y;
    box.x2 = (int16_t) (x + width);
    box.y2 = (int16_t) (y + height);
    res = pixman_region_contains_rectangle(region, &box);
    if (res == PIXMAN_REGION_OUT) {
        return RectangleOut;
    } else if (res == PIXMAN_REGION_IN) {
        return RectangleIn;
    } else {
        return RectanglePart;
    }
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
    
}
