#include "X11/Xlib.h"
#include "SDL_ttf.h"
#include "errors.h"
#include "drawing.h"
#include "colors.h"
#include "resourceTypes.h"

// TODO: Make Cursor to SDL_Cursor*

Cursor createPixmapCursor(Display* display, GPU_Image* source, GPU_Image* mask,
                          XColor* foreground_color, XColor* background_color,
                          unsigned int x, unsigned int y) {
//    if (x > source->w || y > source->h) {
//        fprintf(stderr, "x or y are not in source in XCreatePixmapCursor!\n");
//        handleError(0, display, NULL, 0, BadMatch, XCB_CREATE_CURSOR, 0);
//        return NULL;
//    }
//    if (mask != NULL && (mask->w != source->w || mask->h != source->h)) {
//        fprintf(stderr, "Given mask is not the same size as source in XCreatePixmapCursor!\n");
//        handleError(0, display, NULL, 0, BadMatch, XCB_CREATE_CURSOR, 0);
//        return NULL;
//    }
    Cursor cursor = malloc(sizeof(Cursor));
    if (cursor == NULL) {
        handleOutOfMemory(0, display, 0, XCB_CREATE_CURSOR, 0);
        return NULL;
    }
    // TODO: IMPLEMENT!!!
    cursor->texture = NULL;/*SDL_CreateRGBSurface(0, source->w, source->h, SDL_SURFACE_DEPTH,
                                           DEFAULT_RED_MASK, DEFAULT_GREEN_MASK, DEFAULT_BLUE_MASK,
                                           DEFAULT_ALPHA_MASK);
    if (cursor->surface == NULL) {
        fprintf(stderr, "CreateRGBSurface failed in XCreatePixmapCursor: %s\n", SDL_GetError());
        free(cursor);
        handleOutOfMemory(0, display, 0, XCB_CREATE_CURSOR, 0);
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
    return cursor;
}

Cursor XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask, XColor* foreground_color,
                           XColor* background_color, unsigned int x, unsigned int y) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreatePixmapCursor.html
    TYPE_CHECK(source, PIXMAP, XCB_CREATE_CURSOR, display, NULL);
    GPU_Image* sourcePixmap = GET_PIXMAP_IMAGE(source);
    GPU_Image* maskPixmap = NULL;
    if (mask != None) {
        TYPE_CHECK(mask, PIXMAP, XCB_CREATE_CURSOR, display, NULL);
        maskPixmap = GET_PIXMAP_IMAGE(mask);
    }
    return createPixmapCursor(display, sourcePixmap, maskPixmap, foreground_color,
                              background_color, x, y);
}

Cursor XCreateGlyphCursor(Display* display, Font source_font, Font mask_font,
                          unsigned int source_char, unsigned int mask_char,
                          XColor* foreground_color, XColor* background_color) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XCreateGlyphCursor.html
//    if (!TTF_GlyphIsProvided(source_font, source_char)) {
//        fprintf(stderr, "'source_char' is not defined in given font in XCreateGlyphCursor!\n");
//        handleError(0, display, NULL, 0, BadValue, XCB_CREATE_GLYPH_CURSOR, 0);
//        return NULL;
//    }
//    SDL_Color color;
//    color.r = foreground_color->red;
//    color.g = foreground_color->green;
//    color.b = foreground_color->blue;
//    SDL_Surface* source = TTF_RenderGlyph_Blended(source_font, source_char, color);
//    if (source == NULL) {
//        fprintf(stderr, "TTF_RenderGlyph_Solid returned NULL for source: %s\n", TTF_GetError());
//        handleOutOfMemory(0, display, 0, XCB_CREATE_GLYPH_CURSOR, 0);
//        return NULL;
//    }
//    SDL_Surface* mask = None;
//    if (mask_font != None) {
//        if (!TTF_GlyphIsProvided(mask_font, mask_char)) {
//            fprintf(stderr, "'mask_char' is not defined in given font in XCreateGlyphCursor!\n");
//            handleError(0, display, NULL, 0, BadValue, XCB_CREATE_GLYPH_CURSOR, 0);
//            return NULL;
//        }
//        mask = TTF_RenderGlyph_Solid(mask_font, mask_char, color);
//        if (mask == NULL) {
//            fprintf(stderr, "TTF_RenderGlyph_Solid returned NULL for mask: %s\n", TTF_GetError());
//            handleOutOfMemory(0, display, 0, XCB_CREATE_GLYPH_CURSOR, 0);
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

void XFreeCursor(Display* display, Cursor cursor) {
    // https://tronche.com/gui/x/xlib/pixmap-and-cursor/XFreeCursor.html
    SDL_DestroyTexture(cursor->texture);
    free(cursor);
}

void XDefineCursor(Display* display, Window window, Cursor cursor) {
    // https://tronche.com/gui/x/xlib/window/XDefineCursor.html
    // TODO: This is not really implemented
    if (cursor == NULL) {
        SDL_SetCursor(SDL_GetDefaultCursor());
    } else {
        // TODO: Maybe implement via separate update thread?
        // https://wiki.libsdl.org/SDL_CreateCursor and https://wiki.libsdl.org/SDL_SetCursor
        printf("Warning: Displaying of custom cursor currently not implemented.\n");
    }
}
