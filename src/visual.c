#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "visual.h"
#include "util.h"
#include "SDL.h"
#include "errors.h"

Visual* VISUAL_LIST = NULL;
size_t NUM_VISUALS = 0;


Bool initVisuals() {
    // TODO: Check with sdl for real screen values
    // https://wiki.libsdl.org/SDL_PixelFormatEnum
    if (VISUAL_LIST != NULL) {
        LOG("Warn: Visual memory already allocated!\n");
        return True;
    }
    NUM_VISUALS = 1;
    VISUAL_LIST = malloc(sizeof(Visual) * NUM_VISUALS);
    if (VISUAL_LIST == NULL) {
        LOG("Out of memory: Failed to allocate memory for the visuals!\n");
        return False;
    }
    VISUAL_LIST->ext_data = NULL;
    VISUAL_LIST->visualid = 0;	/* visual id of this visual */
    VISUAL_LIST->CLASS_ATTRIBUTE = TrueColor;
    VISUAL_LIST->red_mask = 0xFF000000;
    VISUAL_LIST->green_mask = 0x00FF0000;
    VISUAL_LIST->blue_mask = 0x0000FF00;
    VISUAL_LIST->bits_per_rgb = sizeof(SDL_Color);
    // TODO: Reconsider this
    VISUAL_LIST->map_entries = 16581375 /* (255 * 255 * 255) */ ;	/* color map entries */
    return True;
}

void freeVisuals() {
    size_t i = 0;
    for (i = 0; i < NUM_VISUALS; i++) {
        if (VISUAL_LIST[i].ext_data != NULL) {
            free(VISUAL_LIST[i].ext_data);
        }
    }
    free(VISUAL_LIST);
    VISUAL_LIST = NULL;
}

Visual *getDefaultVisual(int screenIndex) {
    // TODO
    return &VISUAL_LIST[0];
}

VisualID XVisualIDFromVisual(Visual* visual) {
    // https://tronche.com/gui/x/xlib/window/XVisualIDFromVisual.html
    return visual->visualid;
}

void fillVisualInfo(XVisualInfo* info, Visual* visual) {
    info->visual = visual;
    info->visualid = visual->visualid;
    info->CLASS_ATTRIBUTE = visual->CLASS_ATTRIBUTE;
    info->bits_per_rgb = visual->bits_per_rgb;
    info->colormap_size = visual->map_entries;
    info->red_mask = visual->red_mask;
    info->green_mask = visual->green_mask;
    info->blue_mask = visual->blue_mask;
    info->depth = 64; // TODO
    info->screen = 0; // TODO
}

XVisualInfo* XGetVisualInfo(Display* display, long vinfo_mask,
                            XVisualInfo *vinfo_template, int *nitems_return) {
    // https://tronche.com/gui/x/xlib/utilities/XGetVisualInfo.html
    unsigned int i, end;
    Visual* visual;
    i = 0;
    end = NUM_VISUALS;
    if (vinfo_mask & VisualIDMask) {
        i = MAX(0, vinfo_template->visualid);
        end = i + 1;
    }
    Array visualIds;
    if (!initArray(&visualIds, vinfo_mask == VisualNoMask ? NUM_VISUALS : 1)) {
        handleOutOfMemory(0, display, 0, 0);
        *nitems_return = 0;
        return NULL;
    }
    for (; i < end; i++) {
        visual = &VISUAL_LIST[i];
        if (HAS_VALUE(vinfo_mask, VisualIDMask) &&
                visual->visualid != vinfo_template->visualid) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualScreenMask)) {
            // TODO
        }
        if (HAS_VALUE(vinfo_mask, VisualDepthMask)) {
            // TODO
        }
        if (HAS_VALUE(vinfo_mask, VisualClassMask) &&
                visual->CLASS_ATTRIBUTE != vinfo_template->CLASS_ATTRIBUTE) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualRedMaskMask) &&
                visual->red_mask != vinfo_template->red_mask) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualGreenMaskMask) &&
                visual->green_mask != vinfo_template->green_mask) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualBlueMaskMask) &&
                visual->blue_mask != vinfo_template->blue_mask) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualColormapSizeMask) &&
                visual->map_entries != vinfo_template->colormap_size) { continue; }
        if (HAS_VALUE(vinfo_mask, VisualBitsPerRGBMask) &&
                visual->bits_per_rgb != vinfo_template->bits_per_rgb) { continue; }
        insertArray(&visualIds, (void *) i);
    }
    XVisualInfo* visualInfo = malloc(sizeof(XVisualInfo) * visualIds.length);
    if (visualInfo == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        *nitems_return = 0;
        return NULL;
    }
    for (i = 0; i < visualIds.length; i++) {
        visual = &VISUAL_LIST[(unsigned int) visualIds.array[i]];
        fillVisualInfo(&visualInfo[i], visual);
    }
    *nitems_return = visualIds.length;
    freeArray(&visualIds);
    return visualInfo;
}

Status XMatchVisualInfo(Display* display, int screen, int depth, int clazz,
                        XVisualInfo* vinfo_return) {
    // https://tronche.com/gui/x/xlib/utilities/XMatchVisualInfo.html
    (void) display;
    if (VISUAL_LIST == NULL) {
        LOG("Visuals memory is not initialized in %s!\n", __func__);
        return 0;
    }
    unsigned int i;
    if (screen != 0 && depth != 64) { // TODO
        return 0;
    }
    Visual* visual = NULL;
    for (i = 0; i < NUM_VISUALS; i++) {
        if (VISUAL_LIST[i].CLASS_ATTRIBUTE == clazz) {
            visual = &VISUAL_LIST[i];
            break;
        }
    }
    if (visual == NULL) return 0;
    fillVisualInfo(vinfo_return, visual);
    return 1;
}
