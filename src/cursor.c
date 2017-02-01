#include "X11/Xlib.h"
#include "SDL_ttf.h"
#include "errors.h"
#include "drawing.h"
#include "colors.h"
#include "resourceTypes.h"
#include "display.h"

#define GET_CURSOR(cursorId) ((Cursor_*) GET_XID_VALUE(cursorId))

typedef struct {
    SDL_Texture* texture;
    int hotspot_x, hotspot_y;
} Cursor_;

Cursor createPixmapCursor(Display* display, GPU_Image* source, GPU_Image* mask,
                          _Xconst XColor* foreground_color, _Xconst XColor* background_color,
                          unsigned int x, unsigned int y) {
//    if (x > source->w || y > source->h) {
//        fprintf(stderr, "x or y are not in source in XCreatePixmapCursor!\n");
//        handleError(0, display, None, 0, BadMatch, 0);
//        return NULL;
//    }
//    if (mask != NULL && (mask->w != source->w || mask->h != source->h)) {
//        fprintf(stderr, "Given mask is not the same size as source in XCreatePixmapCursor!\n");
//        handleError(0, display, None, 0, BadMatch, 0);
//        return NULL;
//    }
    XID cursorId = ALLOC_XID();
    if (cursorId == None) {
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    SET_XID_TYPE(cursorId, CURSOR);
    Cursor_* cursor = malloc(sizeof(Cursor_));
    if (cursor == NULL) {
        FREE_XID(cursorId);
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    SET_XID_VALUE(cursorId, cursor);
    // TODO: IMPLEMENT!!!
    cursor->texture = NULL;/*SDL_CreateRGBSurface(0, source->w, source->h, SDL_SURFACE_DEPTH,
                                           DEFAULT_RED_MASK, DEFAULT_GREEN_MASK, DEFAULT_BLUE_MASK,
                                           DEFAULT_ALPHA_MASK);
    if (cursor->surface == NULL) {
        fprintf(stderr, "CreateRGBSurface failed in XCreatePixmapCursor: %s\n", SDL_GetError());
        free(cursor);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    LOCK_SURFACE(cursor->surface);
    unsigned int pixel_x, pixel_y;
    for (pixel_x = 0; pixel_x < source->w; pixel_x++) {
        for (pixel_y = 0; pixel_y < source->h; pixel_y++) {
            if (getPixel(source, pixel_x, pixel_y)
                && (mask == NULL || getPixel(mask, pixel_x, pixel_y))) {
                putPixel(cursor->surface, pixel_x, pixel_y, foreground_color->pixel);
            } else {
                putPixel(cursor->surface, pixel_x, pixel_y, background_color->pixel);
            }
        }
    }
    UNLOCK_SURFACE(cursor->surface);
    */
    cursor->hotspot_x = x;
    cursor->hotspot_y = y;
    return cursorId;
}

Cursor XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask, XColor* foreground_color,
                           XColor* background_color, unsigned int x, unsigned int y) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreatePixmapCursor.html
    SET_X_SERVER_REQUEST(display, X_CreateCursor);
    TYPE_CHECK(source, PIXMAP, display, None);
    GPU_Image* sourcePixmap = GET_PIXMAP_IMAGE(source);
    GPU_Image* maskPixmap = NULL;
    if (mask != None) {
        TYPE_CHECK(mask, PIXMAP, display, None);
        maskPixmap = GET_PIXMAP_IMAGE(mask);
    }
    return createPixmapCursor(display, sourcePixmap, maskPixmap, foreground_color,
                              background_color, x, y);
}

Cursor XCreateGlyphCursor(Display* display, Font source_font, Font mask_font,
                          unsigned int source_char, unsigned int mask_char,
                          XColor _Xconst* foreground_color, XColor _Xconst* background_color) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreateGlyphCursor.html
    SET_X_SERVER_REQUEST(display, X_CreateGlyphCursor);
//    if (!TTF_GlyphIsProvided(source_font, source_char)) {
//        fprintf(stderr, "'source_char' is not defined in given font in XCreateGlyphCursor!\n");
//        handleError(0, display, None, 0, BadValue, 0);
//        return NULL;
//    }
//    SDL_Color color;
//    color.r = foreground_color->red;
//    color.g = foreground_color->green;
//    color.b = foreground_color->blue;
//    SDL_Surface* source = TTF_RenderGlyph_Blended(source_font, source_char, color);
//    if (source == NULL) {
//        fprintf(stderr, "TTF_RenderGlyph_Solid returned NULL for source: %s\n", TTF_GetError());
//        handleOutOfMemory(0, display, 0, 0);
//        return NULL;
//    }
//    SDL_Surface* mask = None;
//    if (mask_font != None) {
//        if (!TTF_GlyphIsProvided(mask_font, mask_char)) {
//            fprintf(stderr, "'mask_char' is not defined in given font in XCreateGlyphCursor!\n");
//            handleError(0, display, None, 0, BadValue, 0);
//            return NULL;
//        }
//        mask = TTF_RenderGlyph_Solid(mask_font, mask_char, color);
//        if (mask == NULL) {
//            fprintf(stderr, "TTF_RenderGlyph_Solid returned NULL for mask: %s\n", TTF_GetError());
//            handleOutOfMemory(0, display, 0, 0);
//            return NULL;
//        }
//    }
    Cursor cursor = createPixmapCursor(display, /*source, mask*/NULL, NULL, foreground_color,
                                       background_color, 0, 0);
//    SDL_FreeSurface(source);
//    if (mask != None) {
//        SDL_FreeSurface(mask);
//    }
    return cursor;
}

int XFreeCursor(Display* display, Cursor cursor) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XFreeCursor.html
    SET_X_SERVER_REQUEST(display, X_FreeCursor);
    TYPE_CHECK(cursor, CURSOR, display, 0);
    SDL_DestroyTexture(GET_CURSOR(cursor)->texture);
    free(GET_CURSOR(cursor));
    FREE_XID(cursor);
    return 1;
}

int XDefineCursor(Display* display, Window window, Cursor cursor) {
    // https://tronche.com/gui/x/xlib/window/XDefineCursor.html
    // TODO: This is not really implemented
    if (cursor == None) {
        SDL_SetCursor(SDL_GetDefaultCursor());
    } else {
        // TODO: Maybe implement via separate update thread?
        // https://wiki.libsdl.org/SDL_CreateCursor and https://wiki.libsdl.org/SDL_SetCursor
        printf("Warning: Displaying of custom cursor currently not implemented.\n");
    }
    return 1;
}
