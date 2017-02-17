#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <inttypes.h>
#include <X11/Xregion.h>
#include "pixman.h"
#include "drawing.h"
#include "resourceTypes.h"

typedef struct pixman_region16* pRegion;
#define GET_REGION(pixmanRegion) ((Region) (void*) pixmanRegion)
#define GET_P_REGION(region) ((pRegion) (void*) region)

// TODO: check boolean returns
int XDestroyRegion(Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XDestroyRegion.html
    pixman_region_fini(GET_P_REGION(region));
    return 1;
}

Region XCreateRegion() {
    // https://tronche.com/gui/x/xlib/utilities/regions/XCreateRegion.html
    pRegion region = malloc(sizeof(struct pixman_region16));
    if (region == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate Region structure in XCreateRegion!\n");
        return NULL;
    }
    pixman_region_init(region);
    return GET_REGION(region);
}

Bool XEmptyRegion(Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XEmptyRegion.html
    return !pixman_region_not_empty(GET_P_REGION(region));
}

int XRectInRegion(Region region, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XRectInRegion.html
    pixman_region_overlap_t res;
    pixman_box16_t box;
    box.x1 = (int16_t) x;
    box.y1 = (int16_t) y;
    box.x2 = (int16_t) (x + width);
    box.y2 = (int16_t) (y + height);
    res = pixman_region_contains_rectangle(GET_P_REGION(region), &box);
    if (res == PIXMAN_REGION_OUT) {
        return RectangleOut;
    } else if (res == PIXMAN_REGION_IN) {
        return RectangleIn;
    } else {
        return RectanglePart;
    }
}

int XClipBox(Region region, XRectangle* rect_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XClipBox.html
    pixman_box16_t* extends = pixman_region_extents(GET_P_REGION(region));
    rect_return->width  = (unsigned short) abs(extends->x2 - extends->x1);
    rect_return->y      = extends->y1;
    rect_return->height = (unsigned short) abs(extends->y2 - extends->y1);
    rect_return->x      = extends->x1;
    return 1;
}

int XIntersectRegion(Region sra, Region srb, Region dr_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XIntersectRegion.html
    return pixman_region_intersect(GET_P_REGION(dr_return), GET_P_REGION(sra), GET_P_REGION(srb)) ? 1 : 0;
}

int XSubtractRegion(Region sra, Region srb, Region dr_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XSubtractRegion.html
    return pixman_region_subtract(GET_P_REGION(dr_return), GET_P_REGION(sra), GET_P_REGION(srb)) ? 1 : 0;
}

int XUnionRectWithRegion(XRectangle *rectangle, Region src_region, Region dest_region_return) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XUnionRectWithRegion.html
    return pixman_region_union_rect(GET_P_REGION(dest_region_return), GET_P_REGION(src_region),
                                    rectangle->x, rectangle->y, rectangle->width, rectangle->height) ? 1 : 0;
}

int XSetRegion(Display* display, GC gc, Region region) {
    // https://tronche.com/gui/x/xlib/utilities/regions/XSetRegion.html
    pixman_box16_t* extends = pixman_region_extents(GET_P_REGION(region));
    int width  = abs(extends->x2 - extends->x1);
    int height = abs(extends->y2 - extends->y1);
    // TODO: Implement (https://opensource.apple.com/source/X11/X11-0.40.2/xc/lib/X11/Region.c.auto.html)
    return 1;
}
