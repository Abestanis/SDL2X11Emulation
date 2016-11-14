#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "SDL.h"
#include <SDL_gpu.h>
#include "windowDebug.h"
#include "resourceTypes.h"

typedef struct {
    Atom property;
    int dataFormat;
    int dataLength;
    Atom type;
    unsigned char* data;
} WindowProperty;

typedef enum {UnMapped, Mapped, MapRequested} MapState;

typedef struct {
    /* Parent window of this window, never NULL (except SCREEN_WINDOW). */
    Window parent;
    /* List of children, must end with NULL, can contain NULL between values, can be NULL if no children exist. */
    Window* children;
    /* Number of Windows that can fit in the currently allocated children list. */
    unsigned int childSpace;
    /* This is the drawing target of the window and its children while it is unmapped. Might be NULL.*/
    GPU_Image* unmappedContent;
    /* 
     * This is the SDL Window handler to the real window of this window.
     * Only set if this window is a mapped top level window.
     */
    SDL_Window* sdlWindow;
    /* The render target of this window. Only set if sdlWindow or unmappedContent is set. */
    GPU_Target* renderTarget;
    /* The position of this window relative to its parent. */
    int x, y;
    /* The dimensions of this window. */
    unsigned int w, h;
    Bool inputOnly;
    Visual* visual;
    Colormap colormap;
    unsigned long backgroundColor;
    Pixmap background; // TODO: Is this even used anywhere?
    int colormapWindowsCount;
    Window* colormapWindows;
    unsigned int propertyCount;
    unsigned int propertySize;
    WindowProperty* properties;
    /* The window name. Only used if this window has a corresponding sdlWindow. */
    char* windowName;
    /* The icon of this window. Only used if this window has a corresponding sdlWindow. */
    SDL_Surface* icon;
    unsigned int borderWidth;
    int depth;
    /* Indicates if this window is Mapped, if mapping it is requested or if it is Unmapped. */
    MapState mapState;
    long eventMask;
    Bool overrideRedirect;
    #ifdef DEBUG_WINDOWS
    /* Random id used for debugging. */
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
#define IS_TOP_LEVEL(window) (window != SCREEN_WINDOW && GET_PARENT(window) == SCREEN_WINDOW)
#define IS_MAPPED_TOP_LEVEL_WINDOW(window) (IS_TOP_LEVEL(window) && GET_WINDOW_STRUCT(window)->sdlWindow != NULL)
#define IS_INPUT_ONLY(window) GET_WINDOW_STRUCT(window)->inputOnly
#define GET_WINDOW_POS(window, out_x, out_y) if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {\
    SDL_GetWindowPosition(GET_WINDOW_STRUCT(window)->sdlWindow, &GET_WINDOW_STRUCT(window)->x, &GET_WINDOW_STRUCT(window)->y);\
}\
out_x = GET_WINDOW_STRUCT(window)->x;\
out_y = GET_WINDOW_STRUCT(window)->y

#define GET_WINDOW_DIMS(window, width, height) if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {\
    SDL_GetWindowSize(GET_WINDOW_STRUCT(window)->sdlWindow, &GET_WINDOW_STRUCT(window)->w, &GET_WINDOW_STRUCT(window)->h);\
}\
width = GET_WINDOW_STRUCT(window)->w;\
height = GET_WINDOW_STRUCT(window)->h

#define HAS_VALUE(valueMask, value) (value & valueMask)

#endif /* _WINDOW_H_ */
