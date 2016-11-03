#ifndef _WINDOW_H_
#define _WINDOW_H_

#define DEBUG_WINDOWS 1

#include "SDL.h"
#include "resourceTypes.h"
#include <SDL_gpu.h>
#include "windowDebug.h"

typedef struct {
    Atom property;
    int dataFormat;
    int dataLength;
    Atom type;
    unsigned char* data;
} WindowProperty;

typedef enum {UnMapped, Mapped, MapRequested} MapState;

typedef struct {
    Window parent;
    Window* children; /* List of children, must end with NULL, can contain NULL between values */
    unsigned int childSpace;
    GPU_Image* unmappedContent;
    SDL_Window* sdlWindow;
    GPU_Target* renderTarget;
    int x, y;
    unsigned int w, h;
    Bool inputOnly;
    Visual* visual;
    Colormap colormap;
    unsigned long backgroundColor;
    Pixmap backgroundPixmap;
    int colormapWindowsCount;
    Window* colormapWindows;
    unsigned int propertyCount;
    unsigned int propertySize;
    WindowProperty* properties;
    char* windowName;
    SDL_Surface* icon;
    unsigned int borderWidth;
    int depth;
    MapState mapState;
    long eventMask;
    #ifdef DEBUG_WINDOWS
    unsigned long debugId;
    #endif /* DEBUG_WINDOWS */
} WindowStruct;

#include "windowInternal.h"

void XChangeWindowAttributes(Display* display, Window window, unsigned long valueMask,
                             XSetWindowAttributes *attributes);

extern Window SCREEN_WINDOW;

#define GET_VISUAL(window) GET_WINDOW_STRUCT(window)->visual
#define GET_COLORMAP(window) GET_WINDOW_STRUCT(window)->colormap
#define GET_PARENT(window) GET_WINDOW_STRUCT(window)->parent
#define GET_CHILDREN(window) GET_WINDOW_STRUCT(window)->children
#define IS_ROOT(window) (window != SCREEN_WINDOW && GET_PARENT(window) == SCREEN_WINDOW)
#define IS_MAPPED_TOP_LEVEL_WINDOW(window) (IS_ROOT(window) && GET_WINDOW_STRUCT(window)->sdlWindow != NULL)
#define IS_INPUT_ONLY(window) GET_WINDOW_STRUCT(window)->inputOnly
#define GET_WINDOW_POS(window, out_x, out_y) if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {\
    SDL_GetWindowPosition(GET_WINDOW_STRUCT(window)->sdlWindow, &out_x, &out_y);\
} else {\
    out_x = GET_WINDOW_STRUCT(window)->x;\
    out_y = GET_WINDOW_STRUCT(window)->y;\
}

#define GET_WINDOW_DIMS(window, width, height) if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {\
    SDL_GetWindowSize(GET_WINDOW_STRUCT(window)->sdlWindow, &width, &height);\
} else {\
    width = GET_WINDOW_STRUCT(window)->w;\
    height = GET_WINDOW_STRUCT(window)->h;\
}

#define HAS_VALUE(valueMask, value) (value & valueMask)
#define DEFAULT_TITLE "Untitled"

#endif /* _WINDOW_H_ */
