#include "X11/Xlib.h"
#include "X11/Xatom.h"
#include <stdio.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "errors.h"
#include "colors.h"
#include "resourceTypes.h"
#include "atoms.h"
#include "drawing.h"

// TODO: Convert text decoding to Utf-8
// http://www.cprogramming.com/tutorial/unicode.html

Font XLoadFont(Display* display, char* name) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XLoadFont.html
    int fontSize = 18;
    // TODO: Remove static font size
    // TODO: Implement pattern matching
    // TODO: This function is called with "fixed" and "cursor" as a name
    char* fontName;
    if (strcmp(name, "fixed") == 0 || strcmp(name, "cursor") == 0) {
        // This seems to be a common monospace font on most devices.
        fontName = "/system/fonts/DroidSansMono.ttf";
    } else {
        fontName = name;
    }
    TTF_Font* font = TTF_OpenFont(fontName, fontSize);
    if (font == NULL){
        fprintf(stderr, "Failed to load font %s!\n", name);
        handleError(0, display, NULL, 0, BadName, XCB_OPEN_FONT, 0);
    }
    return font;
}

char** XListFonts(Display* display, char* pattern, int maxnames, int* actual_count_return) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XListFonts.html
    // TODO: Maybe scan trough a default location
    fprintf(stderr, "Hit unimplemented function %s\n", __func__);
    *actual_count_return = 0;
    return NULL;
}

void XFreeFontNames(char* list[]) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XFreeFontNames.html
    // TODO: This wont free the whole list
    free(list);
}

void XFreeFont(Display* display, XFontStruct* font_struct) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XFreeFont.html
    TTF_CloseFont(font_struct->fid);
    if (font_struct->per_char != NULL) {
        int numChars = font_struct->max_char_or_byte2 - font_struct->min_char_or_byte2;
        int i;
        for (i = 0; i < numChars; i++) {
            free(font_struct->per_char + i * sizeof(XCharStruct));
        }
    }
    free(font_struct);
}

char* getFontXLFDName(XFontStruct* font_struct) {
    /* FOUNDRY - FAMILY_NAME - WEIGHT_NAME - SLANT - SETWIDTH_NAME - ADD_STYLE - PIXEL_SIZE -
       POINT_SIZE - RESOLUTION_X - RESOLUTION_Y - SPACING - AVERAGE_WIDTH - CHARSET_REGISTRY -
       CHARSET_ENCODING */
    int fontStyle = TTF_GetFontStyle(font_struct->fid);
    static char* emptyValue = "";
    char* foundry = emptyValue;
    char* familyName = TTF_FontFaceFamilyName(font_struct->fid);
    if (familyName == NULL) familyName = emptyValue;
    char* weightName = fontStyle & TTF_STYLE_BOLD ? "bold" : "medium";
    char slant = fontStyle & TTF_STYLE_ITALIC ? 'i' : 'r';
    char* setWidth = "normal";
    int pointSize = 18 * 10;
    char spacing = TTF_FontFaceIsFixedWidth(font_struct->fid) ? 'm' : 'p';
    short averageWidth = font_struct->max_bounds.width;
    char* charset = "Utf";
    int charsetEncoding = 8;

    char* name = malloc(sizeof(char) * (14 + strlen(foundry) + strlen(familyName) +
            strlen(weightName) + 1 + strlen(setWidth) + 1 + 6 + 1 + 1 + 1 + 5 + strlen(charset) + 1));
    if (name != NULL) {
        sprintf(name, "-%s-%s-%s-%c-%s--0-%d-0-0-%c-%hd-%s-%d", foundry, familyName, weightName,
                slant, setWidth, pointSize, spacing, averageWidth, charset, charsetEncoding);
    }
    fprintf(stderr, "Font name = '%s'\n", name);
    return name;
}

Bool XGetFontProperty(XFontStruct* font_struct, Atom atom, unsigned long* value_return) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XGetFontProperty.html
    Bool res = False;
    char* name;
    switch (atom) {
        case XA_FONT:
            name = getFontXLFDName(font_struct);
            if (name != NULL) {
                *value_return = (unsigned long) internalInternAtom(name);
                res = True;
            }
            break;
        default:
            fprintf(stderr, "%s: Got unknown atom %lu!\n", __func__, atom);
    }
    // Can't provide values for XA_UNDERLINE_POSITION and XA_UNDERLINE_THICKNESS
    return res;
}

Bool fillXCharStruct(TTF_Font* font, unsigned int character, XCharStruct* charStruct) {
    int minX, maxX, minY, maxY, advance;
    if (TTF_GlyphMetrics(font, (Uint16) character, &minX, &maxX, &minY, &maxY, &advance) == -1) {
        fprintf(stderr, "Failed to determine metrics for character '%u': %s\n", character, TTF_GetError());
        return False;
    }
    charStruct->width = advance;
    charStruct->rbearing = maxX;
    return True;
}

XFontStruct* XLoadQueryFont(Display* display, char* name) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XLoadQueryFont.html
    TTF_Font* font = XLoadFont(display, name);
    if (font == NULL) {
        return NULL;
    }
    XFontStruct* fontStruct = malloc(sizeof(XFontStruct));
    if (fontStruct == NULL) {
        handleOutOfMemory(0, display, 0, XCB_QUERY_FONT, 0);
        TTF_CloseFont(font);
        return NULL;
    }
    fontStruct->fid = font;
    fontStruct->ascent = TTF_FontAscent(font);
    fontStruct->descent = TTF_FontDescent(font);
    fontStruct->per_char = NULL;
    unsigned int numChars = 0;
    unsigned int i;
    for (i = 0; i < 65536 /* 2^16 */; i++) {
        if (TTF_GlyphIsProvided(font, (Uint16) i)) {
            if (numChars == 0) {
                fontStruct->min_char_or_byte2 = i;
            }
            fontStruct->max_char_or_byte2 = i;
            numChars++;
        }
    }
//    if (numChars >= 256) {
//        fontStruct->min_byte1 = fontStruct->min_char_or_byte2 / 256;
//        fontStruct->max_byte1 = fontStruct->max_char_or_byte2 / 256;
//        fontStruct->min_char_or_byte2 = 0;
//    } else {
        fontStruct->min_byte1 = 0;
        fontStruct->max_byte1 = 0;
//    }
    // TODO: This is debugging
    fontStruct->max_char_or_byte2 = 255;
    // Build per_char
    int monospace = TTF_FontFaceIsFixedWidth(font);
    XCharStruct charStruct;
    if (!monospace) {
        fontStruct->per_char = malloc(sizeof(XCharStruct) * numChars);
        if (fontStruct->per_char == NULL) {
            handleOutOfMemory(0, display, 0, XCB_QUERY_FONT, 0);
            XFreeFont(display, fontStruct);
            return NULL;
        }
        charStruct = fontStruct->per_char[0];
    }
    if (fillXCharStruct(font, fontStruct->min_char_or_byte2, &charStruct) == False) {
        XFreeFont(display, fontStruct);
        return NULL;
    }
    fontStruct->max_bounds = charStruct;
    fontStruct->min_bounds = charStruct;
    if (monospace) {
        fontStruct->per_char = NULL;
    } else {
        int counter = 1;
        for (i = fontStruct->min_char_or_byte2 + 1; i < 65536 /* 2^16 */; ++i) {
            if (TTF_GlyphIsProvided(font, (Uint16) i)) {
                charStruct = fontStruct->per_char[counter];
                if (fillXCharStruct(font, i, &charStruct) == False) {
                    XFreeFont(display, fontStruct);
                    return NULL;
                }
                // I think rbearing (aka. advance) is the value that matters here
                if (fontStruct->max_bounds.rbearing < charStruct.rbearing) {
                    fontStruct->max_bounds = charStruct;
                } else if (fontStruct->min_bounds.rbearing > charStruct.rbearing) {
                    fontStruct->max_bounds = charStruct;
                }
                counter++;
            }
        }
    }
    return fontStruct;
}

__inline__ int hexCharToNum(char chr) {
    return chr >= 'a' ? 10 + chr - 'a' : chr - '0';
}

/* Resolve all X11 controll characters */
const char* decodeString(char* string, int count) {
    int i, counter = 0;
    char* text = malloc(sizeof(char) * (count + 1));
    if (text == NULL) { return NULL; }
    for (i = 0; i < count; i++) {
        if (string[i] == '\\') {
            switch (string[++i]) {
                case 'x': /* \xFF */
                    text[counter] = (hexCharToNum(string[++i]) << 4);
                    text[counter] |= hexCharToNum(string[++i]);
                    break;
                case 'u': /* \uFFFF */
                    text[counter] = (hexCharToNum(string[++i]) << 4);
                    text[counter] |= hexCharToNum(string[++i]);
                    text[++counter] = (hexCharToNum(string[++i]) << 4);
                    text[counter] |= hexCharToNum(string[++i]);
                    break;
                case 'n':
                    text[counter] = '\n';
                    break;
                case 'r':
                    text[counter] = '\r';
                    break;
                case 'a':
                    text[counter] = '\a';
                    break;
                case 'b':
                    text[counter] = '\b';
                    break;
                case 't':
                    text[counter] = '\t';
                    break;
                case 'v':
                    text[counter] = '\v';
                    break;
                case 'f':
                    text[counter] = '\f';
                    break;
                default:
                    fprintf(stderr, "Warn: Got unknown controll character in %s: '\\%c'\n", __func__, string[i]);
                    text[counter] = '\\';
                    text[++counter] = string[i];
            }
        } else {
            text[counter] = string[i];
        }
        counter++;
    }
    text[counter] = '\0';
    return (const char*) text;
}

int getTextWidth(XFontStruct* font_struct, const char* string) {
    int width, height;
    if (TTF_SizeUTF8(font_struct->fid, string, &width, &height) != 0) {
        fprintf(stderr, "Failed to calculate the text with in XTextWidth[16]: %s! Returning max width of font.\n", TTF_GetError());
        return font_struct->max_bounds.rbearing * strlen(string);
    }
    return width;
}

int XTextWidth16(XFontStruct* font_struct, XChar2b* string, int count) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XTextWidth16.html
    // TODO: Rethink this
    fprintf(stderr, "Hit unimplemented function %s!\n", __func__);
//    Uint16* text = (Uint16*) decodeString((char*) string, count, True);
//    if (text == NULL) {
//        fprintf(stderr, "Out of memory: Failed to allocate memory in XTextWidth16! Returning max width of font.\n");
//        return font_struct->max_bounds.rbearing * count;
//    }
//    return getTextWidth(font_struct, text);
}

int XTextWidth(XFontStruct* font_struct, char* string, int count) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XTextWidth.html
    const char* text = decodeString(string, count);
    if (text == NULL) {
        fprintf(stderr, "Out of memory: Failed to allocate memory in XTextWidth! Returning max width of font.\n");
        return font_struct->max_bounds.rbearing * count;
    }
    int width = getTextWidth(font_struct, text);
    free((char*) text);
    return width;
}

Bool renderText(Display* display, SDL_Renderer* renderer, GC gc, int x, int y, const char* string) {
    fprintf(stderr, "Rendering text: '%s'\n", string);
    if (string == NULL || string[0] == '\0') { return True; }
    SDL_Color color;// = uLongToColor(surface->format, gc->foreground);
    color.r = (gc->foreground & 0xFF000000) >> 24;
    color.g = (gc->foreground & 0x00FF0000) >> 16;
    color.b = (gc->foreground & 0x0000FF00) >> 8;
    color.a = 0xFF;
    SDL_Surface* fontSurface = TTF_RenderUTF8_Blended(gc->font, string, color);
    if (fontSurface == NULL) {
        return False;
    }
    SDL_Rect destR;
    destR.w = fontSurface->w;
    destR.h = fontSurface->h;
    SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);
    if (fontTexture == NULL) {
        return False;
    }
    destR.x = x;
    destR.y = y - TTF_FontAscent(gc->font)/* - 6*/;
    // h and w are ignored
    if (SDL_RenderCopy(renderer, fontTexture, NULL, &destR) != 0) {
        return False;
    }
    SDL_DestroyTexture(fontTexture);
    return True;
}

void XDrawString16(Display* display, Drawable drawable, GC gc, int x, int y, XChar2b* string, int length) {
    // https://tronche.com/gui/x/xlib/graphics/drawing-text/XDrawString16.html
    // TODO: Rethink this
    fprintf(stderr, "%s: Drawing on %p\n", __func__, drawable);
    if (drawable == NULL) {
        handleError(0, display, 0, 0, BadDrawable, XCB_DRAW_STRING_16, 0);
        return;
    }
    TYPE_CHECK(drawable, DRAWABLE, XCB_DRAW_STRING_16, display, );
    if (gc == NULL) {
        handleError(0, display, 0, 0, BadGC, XCB_DRAW_STRING_16, 0);
        return;
    }
    if (length == 0 || ((Uint16*) string)[0] == 0) { return; }
    SDL_Renderer* renderer;
    GET_RENDERER(drawable, renderer);
    fprintf(stderr, "Hit unimplemented function %s!\n", __func__);
//    const XChar2b* text = decodeString((char*) string, length, True);
//    if (text == NULL) {
//        fprintf(stderr, "Out of memory: Failed to allocate memory in XDrawString16, raising BadMatch error.\n");
//        handleError(0, display, drawable, 0, BadMatch, XCB_DRAW_STRING_16, 0);
//        return;
//    }
//    if (!renderText(display, renderer, gc, x, y, (Uint16*) text)) {
//        fprintf(stderr, "Rendering the text failed in %s: %s\n", __func__, SDL_GetError());
//        handleError(0, display, drawable, 0, BadMatch, XCB_DRAW_STRING_16, 0);
//    }
}

void XDrawString(Display* display, Drawable drawable, GC gc, int x, int y, char* string, int length) {
    // https://tronche.com/gui/x/xlib/graphics/drawing-text/XDrawString.html
    fprintf(stderr, "%s: Drawing on %p\n", __func__, drawable);
    if (drawable == NULL) {
        handleError(0, display, 0, 0, BadDrawable, XCB_DRAW_STRING, 0);
        return;
    }
    TYPE_CHECK(drawable, DRAWABLE, XCB_DRAW_STRING, display, );
    if (gc == NULL) {
        handleError(0, display, 0, 0, BadGC, XCB_DRAW_STRING, 0);
        return;
    }
    if (length == 0 || string[0] == 0) { return; }
    SDL_Renderer* renderer;
    GET_RENDERER(drawable, renderer);
    const char* text = decodeString(string, length);
    if (text == NULL) {
        fprintf(stderr, "Out of memory: Failed to allocate decoded string in XDrawString, raising BadMatch error.\n");
        handleError(0, display, 0, 0, BadMatch, XCB_DRAW_STRING, 0);
        return;
    }
    if (!renderText(display, renderer, gc, x, y, text)) {
        free((char*) text);
        fprintf(stderr, "Rendering the text failed in %s: %s\n", __func__, SDL_GetError());
        handleError(0, display, drawable, 0, BadMatch, XCB_DRAW_STRING, 0);
    }
    free((char*) text);
}
