#include "X11/Xatom.h"
#include <stdio.h>
#include <wchar.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <X11/Xlib.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "errors.h"
#include "colors.h"
#include "resourceTypes.h"
#include "atoms.h"
#include "drawing.h"
#include "display.h"
#include "gc.h"
#include "util.h"
#include "font.h"

// TODO: Maybe implement character atlas
// TODO: Convert text decoding to Utf-8
// http://www.cprogramming.com/tutorial/unicode.html

typedef struct {
    char* filePath;
    char* XLFName;
} FontCacheEntry;

#define GET_FONT(fontXID) ((TTF_Font*) GET_XID_VALUE(fontXID))
#define FONT_SIZE 8

// This Array contains a list of default font search paths for the compiled architecture
static const char* DEFAULT_FONT_SEARCH_PATHS[] = {
#ifdef __ANDROID__
        "/system/fonts"
#endif /* __ANDROID__ */
};

// This are the custom search paths for fonts can be set via XSetFontPath.
// Has to be initialized via XSetFontPath(display, NULL, 0);
Array* fontSearchPaths = NULL;
Array* fontCache = NULL;

// Check if the given path points to an existing directory
bool checkFontPath(const char* path) {
    if (path == NULL) return False;
    struct stat s;
    int err;
    while ((err = stat(path, &s)) == -1 && errno == EAGAIN);
    if (err == 0) {
        if(S_ISDIR(s.st_mode)) {
            return True;
        }
    }
    return False;
}

char* getFontXLFDName(TTF_Font* font) {
    /* FOUNDRY - FAMILY_NAME - WEIGHT_NAME - SLANT - SETWIDTH_NAME - ADD_STYLE - PIXEL_SIZE -
       POINT_SIZE - RESOLUTION_X - RESOLUTION_Y - SPACING - AVERAGE_WIDTH - CHARSET_REGISTRY -
       CHARSET_ENCODING */
    int fontStyle = TTF_GetFontStyle(font);
    static char* emptyValue = "";
    char* foundry = emptyValue;
    char* familyName = TTF_FontFaceFamilyName(font);
    if (familyName == NULL) familyName = emptyValue;
    char* weightName = fontStyle & TTF_STYLE_BOLD ? "bold" : "medium";
    char slant = (char) (fontStyle & TTF_STYLE_ITALIC ? 'i' : 'r');
    char* setWidth = "normal";
    int pointSize = 0;
    char spacing = (char) (TTF_FontFaceIsFixedWidth(font) ? 'm' : 'p');
    short averageWidth = 0;
    char* charset = "iso10646"; // Unicode
    int charsetEncoding = 1;
    size_t nameLength = 14 + strlen(foundry) + strlen(familyName) + strlen(weightName)
                        + 1 + strlen(setWidth) + 1 + 6 + 1 + 1 + 1 + 5 + strlen(charset) + 1;
    
    char* name = malloc(sizeof(char) * nameLength);
    if (name != NULL) {
        // TODO: Should this not be "-%s-%s-%s-%c-%s--0-%d-0-0-%c-%hd-%s-%d"? Tk does not want the extra dash.
        snprintf(name, nameLength, "-%s-%s-%s-%c-%s-0-%d-0-0-%c-%hd-%s-%d",
                 foundry, familyName, weightName, slant, setWidth, pointSize,
                 spacing, averageWidth, charset, charsetEncoding);
    }
    LOG("Font name = '%s'\n", name);
    return name;
}

Bool fontCacheEntryFileNameCmp(void* entry, void* name) {
    size_t nameLen = strlen(name);
    size_t pathLen = strlen(((FontCacheEntry*) entry)->filePath);
    if (nameLen > pathLen) return False;
    return strcmp(&((FontCacheEntry*) entry)->filePath[pathLen - nameLen], name) == 0;
}

Bool fontCmp(void* entry, void* fontWildCard) {
    return matchWildcard(fontWildCard, ((FontCacheEntry*) entry)->XLFName);
}

Bool updateFontCache() {
    size_t i, entryNameLen;
    size_t fontCacheIndex = 0;
    DIR* fontDirectory;
    struct dirent* entry;
    char pathBuffer[512];
    for (i = 0; i < fontSearchPaths->length; i++) {
        char* fontDirPath = fontSearchPaths->array[i];
        fontDirectory = opendir(fontDirPath);
        if (fontDirectory == NULL) {
            free(removeArray(fontSearchPaths, i, False));
            i--;
            continue;
        }
        while ((entry = readdir(fontDirectory)) != NULL) {
            // We add all missing fonts to the cache, swapping their position to the front.
            // We can be sure, that the fonts with an index lower than fontCacheIndex are valid.
            entryNameLen = strlen(entry->d_name);
            if (entryNameLen > 4 && strncmp(&entry->d_name[entryNameLen - 4], ".ttf", 4) == 0) {
                ssize_t index = findInArrayNCmp(fontCache, entry->d_name,
                                                fontCacheIndex, &fontCacheEntryFileNameCmp);
                
                if (index == -1) {
                    snprintf(pathBuffer, 512, "%s/%s", fontDirPath, entry->d_name);
                    TTF_Font* font = TTF_OpenFont(pathBuffer, FONT_SIZE);
                    if (font == NULL) continue;
                    FontCacheEntry* fontCacheEntry = malloc(sizeof(FontCacheEntry));
                    if (fontCacheEntry == NULL) {
                        TTF_CloseFont(font);
                        closedir(fontDirectory);
                        return False;
                    }
                    fontCacheEntry->filePath = strdup(pathBuffer);
                    fontCacheEntry->XLFName = getFontXLFDName(font);
                    if (fontCacheEntry->filePath == NULL ||  fontCacheEntry->XLFName == NULL) {
                        if (fontCacheEntry->filePath != NULL) free(fontCacheEntry->filePath);
                        if (fontCacheEntry->XLFName != NULL) free(fontCacheEntry->XLFName);
                        free(fontCacheEntry);
                        TTF_CloseFont(font);
                        closedir(fontDirectory);
                        return False;
                    }
                    if (!insertArray(fontCache, fontCacheEntry)) {
                        free(fontCacheEntry->filePath);
                        free(fontCacheEntry->XLFName);
                        free(fontCacheEntry);
                        TTF_CloseFont(font);
                        closedir(fontDirectory);
                        return False;
                    }
                    index = fontCache->length - 1;
                }
                if (index != fontCacheIndex) {
                    swapArray(fontCache, (size_t) index, fontCacheIndex);
                }
                fontCacheIndex++;
            }
        }
        closedir(fontDirectory);
    }
    while (fontCache->length > fontCacheIndex) {
        // Remove all invalid cache entries
        FontCacheEntry* cacheEntry = removeArray(fontCache, fontCache->length - 1, True);
        free(cacheEntry->filePath);
        free(cacheEntry->XLFName);
        free(cacheEntry);
    }
    return True;
}

Bool initFontStorage() {
    size_t fontCount = 0;
    DIR* fontDirectory;
    struct dirent* entry;
    fontSearchPaths = malloc(sizeof(Array));
    if (fontSearchPaths == NULL) {
        return False;
    }
    if (!initArray(fontSearchPaths, ARRAY_LENGTH(DEFAULT_FONT_SEARCH_PATHS))) {
        return False;
    }
    for (size_t i = 0; i < ARRAY_LENGTH(DEFAULT_FONT_SEARCH_PATHS); i++) {
        const char *path = DEFAULT_FONT_SEARCH_PATHS[i];
        if (checkFontPath(path)) {
            fontDirectory = opendir(path);
            if (fontDirectory == NULL) continue;
            path = strdup(path);
            if (path == NULL) return False;
            insertArray(fontSearchPaths, (char*) path);
            // Count the font files in the directory
            while ((entry = readdir(fontDirectory)) != NULL) {
                if (strncmp(&entry->d_name[strlen(entry->d_name) - 4], ".ttf", 4) == 0) {
                    fontCount++;
                }
            }
            closedir(fontDirectory);
        }
    }
    fontCache = malloc(sizeof(Array));
    if (fontCache == NULL) {
        return False;
    }
    if (!initArray(fontCache, fontCount)) {
        return False;
    }
    return updateFontCache();
}

void freeFontStorage() {
    if (fontSearchPaths != NULL) {
        // Clear the array and free the data
        while (fontSearchPaths->length > 0) {
            free(removeArray(fontSearchPaths, 0, False));
        }
        freeArray(fontSearchPaths);
        free(fontSearchPaths);
        fontSearchPaths = NULL;
    }
    if (fontCache != NULL) {
        // Clear the array and free the data
        while (fontCache->length > 0) {
            FontCacheEntry* entry = removeArray(fontSearchPaths, 0, False);
            free(entry->filePath);
            free(entry->XLFName);
            free(entry);
        }
        freeArray(fontCache);
        free(fontCache);
        fontSearchPaths = NULL;
    }
}

Font XLoadFont(Display* display, _Xconst char* name) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XLoadFont.html
    SET_X_SERVER_REQUEST(display, X_OpenFont);
    XID font = ALLOC_XID();
    if (font == None) {
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    SET_XID_TYPE(font, FONT);
    int fontSize = FONT_SIZE;
    // TODO: Remove static font size
    // TODO: Handle "fixed" and "cursor" and others alike right.
    const char* fontPath = NULL;
    if (strcmp(name, "fixed") == 0 || strcmp(name, "cursor") == 0) {
        // This seems to be a common monospace font on most Android devices.
        fontPath = "/system/fonts/DroidSansMono.ttf";
    } else {
        ssize_t index = findInArrayCmp(fontCache, (void *) name, &fontCmp);
        if (index != -1) {
            fontPath = ((FontCacheEntry*) fontCache->array[index])->filePath;
        }
    }
    if (fontPath == NULL) {
        FREE_XID(font);
        LOG("Font %s not found in cache!\n", name);
        handleError(0, display, None, 0, BadName, 0);
        return None;
    }
    SET_XID_VALUE(font, TTF_OpenFont(fontPath, fontSize));
    if (GET_XID_VALUE(font) == NULL){
        FREE_XID(font);
        LOG("Failed to load font %s!\n", name);
        handleError(0, display, None, 0, BadName, 0);
        return None;
    }
    return font;
}

int XFreeFontPath(char** list) {
    free(list);
    return 1;
}

char** XGetFontPath(Display *display, int* npaths_return) {
    SET_X_SERVER_REQUEST(display, X_GetFontPath);
    *npaths_return = fontSearchPaths->length;
    char** list = malloc(sizeof(char*) * fontSearchPaths->length);
    if (list == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    memcpy(list, fontSearchPaths->array, sizeof(char*) * fontSearchPaths->length);
    return list;
}

int XSetFontPath(Display* display, char** directories, int ndirs) {
    SET_X_SERVER_REQUEST(display, X_SetFontPath);
    char* path;
    size_t i;
    while (fontSearchPaths->length > 0) {
        // Clear the array and free the data
        free(removeArray(fontSearchPaths, 0, False));
    }
    if (directories == NULL || ndirs == 0) {
        // Reset the search path to the default for the compiled platform
        ndirs = ARRAY_LENGTH(DEFAULT_FONT_SEARCH_PATHS);
        directories = (char **) DEFAULT_FONT_SEARCH_PATHS;
    }
    for (i = 0; i < ndirs; i++) {
        if (checkFontPath(directories[i])) {
            path = strdup(directories[i]);
            if (path == NULL) {
                handleOutOfMemory(0, display, 0, 0);
            } else {
                insertArray(fontSearchPaths, path);
            }
        }
    }
    return updateFontCache() ? 1 : 0;
}

char** XListFonts(Display* display, _Xconst char* pattern, int maxnames, int* actual_count_return) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XListFonts.html
    SET_X_SERVER_REQUEST(display, X_ListFonts);
    Array names;
    initArray(&names, 0);
    size_t i;
    for (i = 0; i < fontCache->length; i++) {
        char* name = fontCache->array[i];
        if (matchWildcard(pattern, name)) {
            insertArray(&names, name);
            if (names.length >= maxnames) break;
        }
    }
    *actual_count_return = (int) names.length;
    if (names.length == 0) return NULL;
    char** list = malloc(sizeof(char*) * names.length);
    if (list == NULL) {
        *actual_count_return = 0;
        return NULL;
    }
    memcpy(list, names.array, names.length);
    return list;
}

int XFreeFontNames(char** list) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XFreeFontNames.html
    free(list);
    return 1;
}

int XFreeFont(Display* display, XFontStruct* font_struct) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XFreeFont.html
    SET_X_SERVER_REQUEST(display, X_CloseFont);
    TTF_CloseFont(GET_FONT(font_struct->fid));
    FREE_XID(font_struct->fid);
    if (font_struct->per_char != NULL) {
        int numChars = font_struct->max_char_or_byte2 - font_struct->min_char_or_byte2;
        int i;
        for (i = 0; i < numChars; i++) {
            free(font_struct->per_char + i * sizeof(XCharStruct));
        }
    }
    free(font_struct);
    return 1;
}

Bool XGetFontProperty(XFontStruct* font_struct, Atom atom, unsigned long* value_return) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XGetFontProperty.html
    Bool res = False;
    char* name;
    switch (atom) {
        case XA_FONT:
            name = getFontXLFDName(GET_FONT(font_struct->fid));
            if (name != NULL) {
                *value_return = (unsigned long) internalInternAtom(name);
                res = True;
            }
            break;
        default:
            LOG("%s: Got unknown atom %lu!\n", __func__, atom);
    }
    // Can't provide values for XA_UNDERLINE_POSITION and XA_UNDERLINE_THICKNESS
    return res;
}

Bool fillXCharStruct(TTF_Font* font, unsigned int character, XCharStruct* charStruct) {
    int minX, maxX, minY, maxY, advance;
    if (TTF_GlyphMetrics(font, (Uint16) character, &minX, &maxX, &minY, &maxY, &advance) == -1) {
        LOG("Failed to determine metrics for character '%u': %s\n", character, TTF_GetError());
        return False;
    }
    charStruct->width = (short) advance;
    charStruct->rbearing = (short) maxX;
    return True;
}

XFontStruct* XLoadQueryFont(Display* display, _Xconst char* name) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XLoadQueryFont.html
    Font fontId = XLoadFont(display, name);
    if (fontId == None) {
        return NULL;
    }
    TTF_Font* font = GET_FONT(fontId);
    SET_X_SERVER_REQUEST(display, X_QueryFont);
    XFontStruct* fontStruct = malloc(sizeof(XFontStruct));
    if (fontStruct == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        TTF_CloseFont(font);
        return NULL;
    }
    fontStruct->fid = fontId;
    fontStruct->ascent = TTF_FontAscent(font);
    fontStruct->descent = abs(TTF_FontDescent(font));
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
            handleOutOfMemory(0, display, 0, 0);
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

static __inline__ char hexCharToNum(char chr) {
    return (char) (chr >= 'a' ? 10 + chr - 'a' : chr - '0');
}

/* Resolve all X11 controll characters */
char* decodeString(const char* string, int count) {
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
                    LOG("Warn: Got unknown control character in %s: '\\%c'\n", __func__, string[i]);
                    text[counter] = '\\';
                    text[++counter] = string[i];
            }
        } else {
            text[counter] = string[i];
        }
        counter++;
    }
    text[counter] = '\0';
    return text;
}

int getTextWidth(XFontStruct* font_struct, const char* string) {
    int width, height;
    if (TTF_SizeUTF8(GET_FONT(font_struct->fid), string, &width, &height) != 0) {
        LOG("Failed to calculate the text with in XTextWidth[16]: %s! "
                    "Returning max width of font.\n", TTF_GetError());
        return (int) (font_struct->max_bounds.rbearing * strlen(string));
    }
    return width;
}

char* decodeMbString(const wchar_t* string, size_t* length) {
    *length = wcstombs(NULL, string, 0);
    char* text = malloc(sizeof(char*) * (*length + 1));
    if (text == NULL) {
        return NULL;
    }
    if (wcstombs(text, string, (*length + 1)) == (size_t) -1) {
        free(text);
        return NULL;
    }
    char* decodedText = decodeString(text, *length );
    free(text);
    return decodedText;
}

int XTextWidth16(XFontStruct* font_struct, _Xconst XChar2b* string, int count) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XTextWidth16.html
    // TODO: Rethink this
    size_t length;
    char* text = decodeMbString((const wchar_t *) string, &length);
    if (text == NULL) {
        LOG("Out of memory: Failed to allocate memory in %s! "
                    "Returning max width of font.\n", __func__);
        return font_struct->max_bounds.rbearing * count;
    }
    int width = getTextWidth(font_struct, text);
    free(text);
    return width;
    
}

int XTextWidth(XFontStruct* font_struct, _Xconst char* string, int count) {
    // https://tronche.com/gui/x/xlib/graphics/font-metrics/XTextWidth.html
    char* text = decodeString(string, count);
    if (text == NULL) {
        LOG("Out of memory: Failed to allocate memory in XTextWidth! "
                    "Returning max width of font.\n");
        return font_struct->max_bounds.rbearing * count;
    }
    int width = getTextWidth(font_struct, text);
    free(text);
    return width;
}

Bool renderText(GPU_Target* renderTarget, GC gc, int x, int y, const char* string) {
    LOG("Rendering text: '%s'\n", string);
    if (string == NULL || string[0] == '\0') { return True; }
    GraphicContext* gContext = GET_GC(gc);
    SDL_Color color = {
            GET_RED_FROM_COLOR(gContext->foreground),
            GET_GREEN_FROM_COLOR(gContext->foreground),
            GET_BLUE_FROM_COLOR(gContext->foreground),
            GET_ALPHA_FROM_COLOR(gContext->foreground),
    };
    SDL_Surface* fontSurface = TTF_RenderUTF8_Blended(GET_FONT(gContext->font), string, color);
    if (fontSurface == NULL) {
        return False;
    }
    GPU_Image* fontImage = GPU_CopyImageFromSurface(fontSurface);
    SDL_FreeSurface(fontSurface);
    if (fontImage == NULL) {
        return False;
    }
    y -= TTF_FontAscent(GET_FONT(gContext->font));
    GPU_Blit(fontImage, NULL, renderTarget, x + fontImage->w / 2, y + fontImage->h / 2);
    GPU_FreeImage(fontImage);
    GPU_Flip(renderTarget);
    return True;
}

int XDrawString16(Display* display, Drawable drawable, GC gc, int x, int y, _Xconst XChar2b* string, int length) {
    // https://tronche.com/gui/x/xlib/graphics/drawing-text/XDrawString16.html
    SET_X_SERVER_REQUEST(display, X_PolyText16);
    LOG("%s: Drawing on %lu\n", __func__, drawable);
    TYPE_CHECK(drawable, DRAWABLE, display, 0);
    if (gc == NULL) {
        handleError(0, display, None, 0, BadGC, 0);
        return 0;
    }
    if (length == 0 || ((Uint16*) string)[0] == 0) { return 1; }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(drawable, renderTarget);
    if (renderTarget == NULL) {
        LOG("Failed to get the render target in %s\n", __func__);
        handleError(0, display, None, 0, BadDrawable, 0);
        return 0;
    }
    size_t size;
    char * text = decodeMbString((const wchar_t *) string, &size);
    if (text == NULL) {
        LOG("Out of memory: Failed to allocate memory in XDrawString16, raising BadMatch error.\n");
        handleError(0, display, drawable, 0, BadMatch, 0);
        return 0;
    }
    int res = 1;
    if (!renderText(renderTarget, gc, x, y, text)) {
        LOG("Rendering the text failed in %s: %s\n", __func__, SDL_GetError());
        handleError(0, display, drawable, 0, BadMatch, 0);
        free(text);
        res = 0;
    }
    free(text);
    return res;
}

int XDrawString(Display* display, Drawable drawable, GC gc, int x, int y, _Xconst char* string, int length) {
    // https://tronche.com/gui/x/xlib/graphics/drawing-text/XDrawString.html
    SET_X_SERVER_REQUEST(display, X_PolyText8);
    LOG("%s: Drawing on %lu\n", __func__, drawable);
    TYPE_CHECK(drawable, DRAWABLE, display, 0);
    if (gc == NULL) {
        handleError(0, display, None, 0, BadGC, 0);
        return 0;
    }
    if (length == 0 || string[0] == 0) { return 1; }
    GPU_Target* renderTarget;
    GET_RENDER_TARGET(drawable, renderTarget);
    if (renderTarget == NULL) {
        LOG("Failed to get the render target in %s\n", __func__);
        handleError(0, display, None, 0, BadDrawable, 0);
        return 0;
    }
    char* text = decodeString(string, length);
    if (text == NULL) {
        LOG("Out of memory: Failed to allocate decoded string in XDrawString, "
                    "raising BadMatch error.\n");
        handleError(0, display, None, 0, BadMatch, 0);
        return 0;
    }
    int res = 1;
    if (!renderText(renderTarget, gc, x, y, text)) {
        LOG("Rendering the text failed in %s: %s\n", __func__, SDL_GetError());
        handleError(0, display, drawable, 0, BadMatch, 0);
        res = 0;
    }
    free(text);
    return res;
}
