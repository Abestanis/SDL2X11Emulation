#include "X11/Xlib.h"
#include "window.h"
#include "errors.h"
#include "drawing.h"
#include "atoms.h"
#include "netAtoms.h"
#include "events.h"

#ifdef DEBUG_WINDOWS
# include <stdlib.h>
#endif /* DEBUG_WINDOWS */

// TODO: Cover cases where top-level window is re-parented and window is converted to top-level window
// TODO: Make child list ordered and with no NULL objects

Window SCREEN_WINDOW = NULL;

void initWindowStruct(WindowStruct* windowStruct, int x, int y, unsigned int width, unsigned int height,
                      Visual* visual, Colormap colormap, Bool inputOnly,
                      unsigned long backgroundColor, Pixmap backgroundPixmap) {
    windowStruct->parent = NULL;
    windowStruct->children = NULL;
    windowStruct->childSpace = 0;
    windowStruct->x = x;
    windowStruct->y = y;
    windowStruct->w = width;
    windowStruct->h = height;
    windowStruct->inputOnly = inputOnly;
    windowStruct->colormap = colormap;
    windowStruct->visual = visual;
    windowStruct->sdlTexture = NULL;
    windowStruct->sdlWindow = NULL;
    windowStruct->sdlRenderer = NULL;
    windowStruct->backgroundColor = backgroundColor;
    windowStruct->backgroundPixmap = backgroundPixmap;
    windowStruct->colormapWindowsCount = -1;
    windowStruct->colormapWindows = NULL;
    windowStruct->propertyCount = 0;
    windowStruct->propertySize = 0;
    windowStruct->properties = NULL;
    windowStruct->windowName = NULL;
    windowStruct->icon = NULL;
    windowStruct->borderWidth = 0;
    windowStruct->depth = 0;
    windowStruct->mapState = UnMapped;
    windowStruct->eventMask = 0;
    #ifdef DEBUG_WINDOWS
    windowStruct->debugId = (rand() << 16) | rand();
    #endif /* DEBUG_WINDOWS */
}

/* Screen window handles */

Bool initScreenWindow(Display* display) {
    if (SCREEN_WINDOW == NULL) {
        SCREEN_WINDOW = malloc(sizeof(Window));
        if (SCREEN_WINDOW == NULL) {
            fprintf(stderr, "Out of memory: Failed to allocate SCREEN_WINDOW in initScreenWindow!\n");
            return False;
        }
        SCREEN_WINDOW->type = WINDOW;
        WindowStruct* window = malloc(sizeof(WindowStruct));
        if (window == NULL) {
            free(SCREEN_WINDOW);
            SCREEN_WINDOW = NULL;
            fprintf(stderr, "Out of memory: Failed to allocate SCREEN_WINDOW in initScreenWindow!\n");
            return False;
        }
        initWindowStruct(window, 0, 0, display->screens[0].width, display->screens[0].height,
                         NULL, NULL, False, 0, NULL);
        SCREEN_WINDOW->dataPointer = window;
//        window->sdlWindow = SDL_CreateWindow("Internal", SDL_WINDOWPOS_UNDEFINED,
//                                             SDL_WINDOWPOS_UNDEFINED, 1, 1,
//                                             SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
//        if (window->sdlWindow == NULL) {
//            fprintf(stderr, "Creating the main internal window failed: %s\n", SDL_GetError());
//            free(SCREEN_WINDOW);
//            SCREEN_WINDOW = NULL;
//            return False;
//        }
//        window->sdlRenderer = SDL_CreateRenderer(window->sdlWindow, -1, SDL_RENDERER_TARGETTEXTURE);
        SDL_Surface* sdlSurface = SDL_CreateRGBSurface(0, 1, 1, SDL_SURFACE_DEPTH, DEFAULT_RED_MASK,
                                                       DEFAULT_GREEN_MASK, DEFAULT_BLUE_MASK,
                                                       DEFAULT_ALPHA_MASK);
        window->sdlRenderer = SDL_CreateSoftwareRenderer(sdlSurface);
        if (window->sdlRenderer == NULL) {
            fprintf(stderr, "Creating the main renderer failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(window->sdlWindow);
            window->sdlWindow = NULL;
            free(SCREEN_WINDOW);
            SCREEN_WINDOW = NULL;
            return False;
        }
        window->mapState = Mapped;
    }
    return True;
}

void destroyScreenWindow(Display* display) {
    if (SCREEN_WINDOW != NULL) {
        free(SCREEN_WINDOW->dataPointer);
        free(SCREEN_WINDOW);
        SCREEN_WINDOW = NULL;
    }
}

/* Utility methods */

WindowSdlIdMapper* mappingListStart = NULL;

WindowSdlIdMapper* getWindowSdlIdMapperStructFromId(Uint32 sdlWindowId) {
    WindowSdlIdMapper* mapper;
    for (mapper = mappingListStart; mapper != NULL; mapper = mapper->next) {
        if (mapper->sdlWindowId == sdlWindowId) { return mapper; }
    }
    return NULL;
}

void registerWindowMapping(Window window, Uint32 sdlWindowId) {
    WindowSdlIdMapper* mapper = getWindowSdlIdMapperStructFromId(sdlWindowId);
    if (mapper == NULL) {
        mapper = malloc(sizeof(WindowSdlIdMapper));
        if (mapper == NULL) {
            fprintf(stderr, "Failed to allocate mapping object to map xWindow to SDL window ID!\n");
            return;
        }
        mapper->next = mappingListStart;
        mappingListStart = mapper;
        mapper->sdlWindowId = sdlWindowId;
    }
    mapper->window = window;
}

Window getWindowFromId(Uint32 sdlWindowId) {
    WindowSdlIdMapper* mapper = getWindowSdlIdMapperStructFromId(sdlWindowId);
    return mapper == NULL ? None : mapper->window;
}

#ifdef DEBUG_WINDOWS

#include <unistd.h>

void printWindowHierarchyOfChild(Window window, char* prepend, int prependLen) {
    Window* children = GET_CHILDREN(window);
    const static char* WINDOW_STATES[] = {[Mapped] = "Mapped", [MapRequested] = "MapRequested", [UnMapped] = "UnMapped"};
    char* childPrepend = malloc(sizeof(char) * (prependLen + 2));
    int childCounter = 0;
    int i;
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) childCounter++;
    }
    strcpy(childPrepend, prepend);
    char* charPointer = childPrepend + prependLen;
    *(charPointer + 1) = '\0';
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            int x, y, w, h;
            GET_WINDOW_POS(children[i], x, y);
            GET_WINDOW_DIMS(children[i], w, h);
            printf("%s+- Window (adress: %p, id: 0x%08lx, x: %d, y: %d, %dx%d, state: %s)",
                   prepend, children[i], GET_WINDOW_STRUCT(children[i])->debugId, x, y, w, h,
                   WINDOW_STATES[GET_WINDOW_STRUCT(children[i])->mapState]);
            if (GET_WINDOW_STRUCT(children[i])->sdlRenderer != NULL) {
                printf(", renderer = %p", GET_WINDOW_STRUCT(children[i])->sdlRenderer);
            }
            if (GET_WINDOW_STRUCT(children[i])->sdlWindow != NULL) {
                printf(", sdlWindow = %d", SDL_GetWindowID(GET_WINDOW_STRUCT(children[i])->sdlWindow));
            }
            if (GET_WINDOW_STRUCT(children[i])->sdlTexture != NULL) {
                int w, h;
                SDL_QueryTexture(GET_WINDOW_STRUCT(children[i])->sdlTexture, NULL, NULL, &w, &h);
                printf(", sdlTexture = %p (%d x %d)", GET_WINDOW_STRUCT(children[i])->sdlTexture, w, h);
            }
            printf("\n");
            *charPointer = childCounter == 1 ? ' ' : '|';
            fflush(stdout);
            printWindowHierarchyOfChild(children[i], childPrepend, prependLen + 1);
            childCounter--;
        }
    }
    free(childPrepend);
}

void printWindowHierarchy() {
    printf("- SCREEN_WINDOW (adress: %p, id = 0x%08lx)\n", SCREEN_WINDOW, GET_WINDOW_STRUCT(SCREEN_WINDOW)->debugId);
    printWindowHierarchyOfChild(SCREEN_WINDOW, "", 0);
    fflush(stdout);
}

#endif /* DEBUG_WINDOWS */

Window getContainingWindow(Window window, int x, int y) {
    int i, child_x, child_y, child_w, child_h;
    Window* childern = GET_CHILDREN(window);
    for (i = GET_WINDOW_STRUCT(window)->childSpace - 1; i >= 0 ; i--) {
        if (childern[i] == NULL) continue;
        GET_WINDOW_POS(childern[i], child_x, child_y);
        GET_WINDOW_DIMS(childern[i], child_w, child_h);
        if (x >= child_x && x <= child_x + child_w && y >= child_y && y <= child_y + child_h) {
            return getContainingWindow(childern[i], x - child_x, y - child_y);
        }
    }
    return window;
}

void removeChildFromParent(Window child) {
    if (child == SCREEN_WINDOW) { return; }
    Window parent = GET_PARENT(child);
    if (parent != NULL) {
        unsigned int parentChildSpace = GET_WINDOW_STRUCT(parent)->childSpace;
        Window* childPointer = GET_CHILDREN(parent);
        int i;
        for (i = 0; i < parentChildSpace; i++) {
            fflush(stderr);
            if (childPointer[i] == child) {
                childPointer[i] = NULL;
            }
        }
    }
}

void destroyWindow(Display* display, Window window, Bool freeParentData) {
    if (window == NULL || window == SCREEN_WINDOW) { return; }
    int i;
    if (freeParentData) {
        removeChildFromParent(window);
    }
    XFreeColormap(display, GET_COLORMAP(window));
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->childSpace > 0) {
        Window* childPointer = GET_CHILDREN(window);
        for (i = 0; i < windowStruct->childSpace; i++) {
            if (childPointer[i] != NULL) {
                destroyWindow(display, childPointer[i], False);
            }
        }
        free(childPointer);
    }
    if (windowStruct->propertySize > 0) {
        WindowProperty* propertyPointer = windowStruct->properties;
        free(propertyPointer);
    }
    if (windowStruct->backgroundPixmap != NULL && windowStruct->backgroundPixmap != None) {
        XFreePixmap(display, windowStruct->backgroundPixmap);
    }
    if (windowStruct->windowName != NULL) {
        free(windowStruct->windowName);
    }
    if (windowStruct->icon != NULL) {
            SDL_FreeSurface(windowStruct->icon);
        }
    if (windowStruct->sdlWindow != NULL) {
        SDL_DestroyWindow(windowStruct->sdlWindow);
    }
    if (windowStruct->sdlRenderer != NULL) {
        SDL_DestroyRenderer(windowStruct->sdlRenderer);
    }
    if (windowStruct->sdlTexture != NULL) {
        SDL_DestroyTexture(windowStruct->sdlTexture);
    }
    free(windowStruct);
    free(window);
    // TODO: DestroyNotify Event
}

void print_bytes(const void *object, size_t size)
{
    size_t i;

    printf("[ ");
    for(i = 0; i < size; i++)
    {
        printf("%02x ", ((const unsigned char *) object)[i] & 0xff);
    }
    printf("]\n");
    fflush(stdout);
}

#include <unistd.h>

Bool increaseChildList(Window window) {
    int i;
    int oldChildSpace = GET_WINDOW_STRUCT(window)->childSpace;
    Window* children = GET_CHILDREN(window);
    int newChildSpace = oldChildSpace == 0 ? 8 : oldChildSpace * 2;
    Window* newSpace = malloc(sizeof(Window) * newChildSpace);
    if (newSpace == NULL) {
        return False;
    }
    memcpy(newSpace, children, sizeof(Window) * oldChildSpace);
    memset(newSpace + oldChildSpace, (int) NULL, sizeof(Window) * (newChildSpace - oldChildSpace));
    fprintf(stderr, "oldChildSpace: %d\n", oldChildSpace);
    for (i = 0; i < oldChildSpace; i++) {
        print_bytes(children + i, sizeof(Window));
    }
    free(children);
    GET_WINDOW_STRUCT(window)->childSpace = newChildSpace;
    GET_WINDOW_STRUCT(window)->children = newSpace;
    return True;
}

// TODO: Improve
Bool addChildToWindow(Window parent, Window child) {
    int parentSpace = GET_WINDOW_STRUCT(parent)->childSpace;
    Window* parentChildren = GET_CHILDREN(parent);
    int startOffset = 0;
    if (parentSpace == 0 || *(parentChildren + parentSpace - 1) != NULL) {
        startOffset = parentSpace;
        if (!increaseChildList(parent)) {
            return False;
        }
        parentSpace = GET_WINDOW_STRUCT(parent)->childSpace;
    }
    parentChildren = GET_CHILDREN(parent);
    parentChildren += startOffset;
    while (*parentChildren != NULL) { parentChildren++; }
    *parentChildren = child;
    if (*(GET_CHILDREN(parent) + parentSpace - 1) != NULL) { // There must be at least one NULL entity at the end
        if (!increaseChildList(parent)) {
            *parentChildren = NULL;
            return False;
        }
    }
    GET_WINDOW_STRUCT(child)->parent = parent;
    return True;
}

Bool isParent(Window window1, Window window2) {
    Window parent = GET_PARENT(window2);
    while (parent != NULL) {
        if (parent == window1) {
            return True;
        }
        parent = GET_PARENT(parent);
    }
    return False;
}

WindowProperty* increasePropertySize(WindowProperty* properties, unsigned int currSize,
                                     unsigned int* newSize) {
    if (currSize == 0) {
        *newSize = 8;
    } else {
        *newSize = currSize * 2;
    }
    WindowProperty* newProperties = malloc(sizeof(WindowProperty) * *newSize);
    if (newProperties == NULL) { return NULL; }
    if (currSize != 0) {
        memcpy(newProperties, properties, sizeof(WindowProperty) * currSize);
    }
    // TODO: Free old properties?
    return newProperties;
}

WindowProperty* findProperty(WindowProperty* properties, unsigned int numProperties, Atom property) {
    int i;
    for (i = 0; i < numProperties; i++) {
        if (properties->property == property) {
            return properties;
        }
        properties++;
    }
    return NULL;
}

void resizeWindowTexture(Window window) {
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->sdlTexture != NULL) {
        SDL_Texture* oldTexture = windowStruct->sdlTexture;
        SDL_Rect destRect;
        destRect.x = 0;
        destRect.y = 0;
        SDL_QueryTexture(oldTexture, NULL, NULL, &destRect.w, &destRect.h);
        windowStruct->sdlTexture = NULL;
        SDL_Renderer* windowRenderer;
        GET_RENDERER(window, windowRenderer);
        SDL_RenderCopy(windowRenderer, oldTexture, NULL, &destRect);
        SDL_DestroyTexture(oldTexture);
    }
}

void XDestroyWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XDestroyWindow.html
    TYPE_CHECK(window, WINDOW, XCB_DESTROY_WINDOW, display, );
    destroyWindow(display, window, True);
}

Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width,
                     unsigned int height, unsigned int border_width, int depth, unsigned int class,
                     Visual* visual, unsigned long valueMask, XSetWindowAttributes* attributes) {
    // https://tronche.com/gui/x/xlib/window/XCreateWindow.html
    TYPE_CHECK(parent, WINDOW, XCB_CREATE_WINDOW, display, NULL);
    Bool inputOnly = (class == InputOnly || (class == CopyFromParent && IS_INPUT_ONLY(parent)));
    if (inputOnly && border_width != 0) {
        fprintf(stderr, "Bad argument: Given class is InputOnly but border_with is not 0 in XCreateWindow!\n");
        handleError(0, display, NULL, 0, BadMatch, XCB_CREATE_WINDOW, 0);
        return NULL;
    }
    Window windowID = malloc(sizeof(Window));
    if (windowID == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window id in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, XCB_CREATE_WINDOW, 0);
        return NULL;
    }
    WindowStruct* windowStruct = malloc(sizeof(WindowStruct));
    if (windowStruct == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window struct in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, XCB_CREATE_WINDOW, 0);
        free(windowID);
        return NULL;
    }
    windowID->type = WINDOW;
    windowID->dataPointer = windowStruct;
    // TODO: Window struct not initialized
    int visualClass;
#if defined(__cplusplus) || defined(c_plusplus)
    visualClass = visual->c_class;
#else
    visualClass = visual->class;
#endif
    Colormap windowColormap = (Colormap) XCreateColormap(display, windowID, visual,
                                                         visualClass == StaticGray ||
                                                         visualClass == StaticColor ||
                                                         visualClass == TrueColor ?
                                                         AllocAll : AllocNone);
    if (windowColormap == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window colormap in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, XCB_CREATE_WINDOW, 0);
        free(windowStruct);
        free(windowID);
        return NULL;
    }
    // FIXME: Warning: Colormap is not initialized!
    // TODO: depth? border_width?
    initWindowStruct(windowStruct, x, y, width, height, visual, windowColormap, inputOnly, 0, NULL);
    windowStruct->depth = depth;
    windowStruct->borderWidth = border_width;
    if (!addChildToWindow(parent, windowID)) {
        fprintf(stderr, "Out of memory: Could not increase size of parent's child list in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, XCB_CREATE_WINDOW, 0);
        XDestroyWindow(display, windowID);
        return NULL;
    }
    if (valueMask != 0) {
        XChangeWindowAttributes(display, windowID, valueMask, attributes);
    }
    XEvent event;
    event.type = CreateNotify;
    event.xcreatewindow.type = CreateNotify;
    event.xcreatewindow.serial = 0;
    event.xcreatewindow.send_event = False;
    event.xcreatewindow.display = display;
    event.xcreatewindow.parent = parent;
    event.xcreatewindow.window = windowID;
    event.xcreatewindow.x = x;
    event.xcreatewindow.y = y;
    event.xcreatewindow.width = width;
    event.xcreatewindow.height = height;
    event.xcreatewindow.border_width = border_width;
    event.xcreatewindow.override_redirect = False;
    event.xany.serial = 0;
    event.xany.display = display;
    event.xany.send_event = False;
    event.xany.type = CreateNotify;
    event.xany.window = windowID;
    enqueueEvent(display, &event);
    return windowID;
}

void XConfigureWindow(Display* display, Window window, unsigned int value_mask,
                      XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/window/XConfigureWindow.html
    TYPE_CHECK(window, WINDOW, XCB_CONFIGURE_WINDOW, display, );
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    Bool isMappedTopLevelWindow = IS_MAPPED_TOP_LEVEL_WINDOW(window);
    if (HAS_VALUE(value_mask, CWX) || HAS_VALUE(value_mask, CWY)) {
        int x, y;
        GET_WINDOW_POS(window, x, y);
        if (HAS_VALUE(value_mask, CWX)) {
            x = values->x;
        }
        if (HAS_VALUE(value_mask, CWY)) {
            y = values->y;
        }
        if (isMappedTopLevelWindow) {
            SDL_SetWindowPosition(windowStruct->sdlWindow, x, y);
        }
        windowStruct->x = x;
        windowStruct->y = y;
    }
    if (HAS_VALUE(value_mask, CWWidth) || HAS_VALUE(value_mask, CWHeight)) {
        int width, height;
        GET_WINDOW_DIMS(window, width, height);
        if (HAS_VALUE(value_mask, CWWidth)) {
            width = values->x;
        }
        if (HAS_VALUE(value_mask, CWHeight)) {
            height = values->y;
        }
        if (isMappedTopLevelWindow) {
            SDL_SetWindowSize(windowStruct->sdlWindow, width, height);
        }
        windowStruct->w = width;
        windowStruct->h = height;
        resizeWindowTexture(window);
    }
    // TODO: Implement re-stacking: https://tronche.com/gui/x/xlib/window/configure.html#XWindowChanges
}

Status XReconfigureWMWindow(Display* display, Window window, int screen_number,
                            unsigned int value_mask, XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XReconfigureWMWindow.html
    XConfigureWindow(display, window, value_mask, values);
    return 1;
}

Status XIconifyWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XIconifyWindow.html
    TYPE_CHECK(window, WINDOW, XCB_ICONIFY_WINDOW, display, 0);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_MinimizeWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
    return 1;
}

Status XSetWMColormapWindows(Display* display, Window window, Window* colormap_windows, int count) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetWMColormapWindows.html
    // TODO: Should we do sth. with the information?
    TYPE_CHECK(window, WINDOW, XCB_SET_WM_COLOR_MAPS_WINDOWS, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    windowStruct->colormapWindowsCount = count;
    windowStruct->colormapWindows = colormap_windows;
    return 1;
}

Status XGetWMColormapWindows(Display* display, Window window, Window** colormap_windows_return, int* count_return) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XGetWMColormapWindows.html
    TYPE_CHECK(window, WINDOW, XCB_GET_WM_COLOR_MAPS_WINDOWS, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->colormapWindowsCount > -1) {
        *count_return = windowStruct->colormapWindowsCount;
        *colormap_windows_return = windowStruct->colormapWindows;
        return 1;
    }
    return 0;
}

void XSetTransientForHint(Display* display, Window window, Window prop_window) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetTransientForHint.html
    // TODO: Ignored for now
}

Window getParentWithEventBit(Window window, long eventBit) {
    for (window = GET_PARENT(window); window != SCREEN_WINDOW; window = GET_PARENT(window)) {
        if (GET_WINDOW_STRUCT(window)->eventMask & eventBit) {
            return window;
        }
    }
    return None;
}

Bool mergeWindowDrawables(Window parent, Window child) {
    WindowStruct* childWindowStruct = GET_WINDOW_STRUCT(child);
    if (childWindowStruct->sdlTexture == NULL) { return True; }
    SDL_Renderer* parentRenderer = getWindowRenderer(parent);
    if (childWindowStruct->sdlRenderer != NULL) {
        SDL_RenderPresent(childWindowStruct->sdlRenderer);
    }
    SDL_Rect destRect;
    GET_WINDOW_POS(child, destRect.x, destRect.y);
    GET_WINDOW_DIMS(child, destRect.w, destRect.h);
    if (SDL_RenderCopy(parentRenderer, childWindowStruct->sdlTexture, NULL, &destRect) != 0) {
        return False;
    }
    SDL_DestroyTexture(childWindowStruct->sdlTexture);
    childWindowStruct->sdlTexture = NULL;
    if (childWindowStruct->sdlRenderer != NULL) {
        SDL_DestroyRenderer(childWindowStruct->sdlRenderer);
        childWindowStruct->sdlRenderer = NULL;
    }
    return True;
}

Window enqueueMapEvent(Display* display, Window window, Window parentWithSubstructureRedirect, Bool searchForSRParent) {
    if (searchForSRParent) {
        parentWithSubstructureRedirect = getParentWithEventBit(window, SubstructureRedirectMask);
    }
    XEvent event;
    if (parentWithSubstructureRedirect == None) {
        event.type = MapNotify;
        event.xmap.type = MapNotify;
        event.xmap.serial = 0;
        event.xmap.send_event = False;
        event.xmap.display = display;
        event.xmap.window = window;
        event.xmap.event = window;
        event.xmap.override_redirect = False;
        event.xany.serial = 0;
        event.xany.display = display;
        event.xany.send_event = False;
        event.xany.type = MapNotify;
        event.xany.window = window;
    } else {
        event.type = MapRequest;
        event.xmaprequest.type = MapRequest;
        event.xmaprequest.serial = 0;
        event.xmaprequest.send_event = False;
        event.xmaprequest.display = display;
        event.xmaprequest.parent = parentWithSubstructureRedirect;
        event.xmaprequest.window = window;
        event.xany.serial = 0;
        event.xany.display = display;
        event.xany.send_event = False;
        event.xany.type = MapRequest;
        event.xany.window = window;
    }
    enqueueEvent(display, &event);
}

void mapRequestedChildren(Display* display, Window window, Window subStructureRedirectParent) {
    Window* children = GET_CHILDREN(window);
    int i;
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (*children != NULL && GET_WINDOW_STRUCT(*children)->mapState == MapRequested) {
            enqueueMapEvent(display, window, subStructureRedirectParent, False);
            GET_WINDOW_STRUCT(*children)->mapState = Mapped;
            mapRequestedChildren(display, *children, subStructureRedirectParent);
        }
        children++;
    }
}

void XMapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XMapWindow.html
    TYPE_CHECK(window, WINDOW, XCB_MAP_WINDOW, display, );
    Window parentWithSubstructureRedirect = None;
    if (GET_WINDOW_STRUCT(window)->mapState == Mapped || GET_WINDOW_STRUCT(window)->mapState == MapRequested) { return; }
    if (IS_ROOT(window)) {
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) { return; }
        fprintf(stderr, "Mapping Window %p\n", window);
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (windowStruct->borderWidth == 0) {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        SDL_Window* sdlWindow = SDL_CreateWindow(windowStruct->windowName != NULL ?
                                                 windowStruct->windowName : DEFAULT_TITLE,
                                                 windowStruct->x, windowStruct->y,
                                                 windowStruct->w, windowStruct->h, flags);
        if (sdlWindow == NULL) {
            fprintf(stderr, "SDL_CreateWindow failed in XMapWindow: %s\n", SDL_GetError());
            handleError(0, display, NULL, 0, BadMatch, XCB_MAP_WINDOW, 0);
            return;
        }
        registerWindowMapping(window, SDL_GetWindowID(sdlWindow));
        SDL_Texture* windowTexture = windowStruct->sdlTexture;
        if (windowTexture != NULL) {
            SDL_Renderer *newRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
            if (newRenderer != NULL) {
                SDL_Renderer* oldWindowRenderer;
                GET_RENDERER(window, oldWindowRenderer);
                SDL_Surface* windowSurface = getRenderSurface(oldWindowRenderer);
                SDL_Texture* oldWindowTexture = SDL_CreateTextureFromSurface(newRenderer, windowSurface);
                SDL_FreeSurface(windowSurface);
                if (SDL_RenderCopy(newRenderer, oldWindowTexture, NULL, NULL) != 0) {
                    fprintf(stderr, "Failed to copy window surface with renderer in XMapWindow: %s\n",
                            SDL_GetError());
                    handleError(0, display, NULL, 0, BadMatch, XCB_MAP_WINDOW, 0);
                    SDL_DestroyWindow(sdlWindow);
                    SDL_DestroyTexture(oldWindowTexture);
                    SDL_DestroyRenderer(newRenderer);
                    return;
                }
                SDL_DestroyTexture(windowTexture);
                SDL_DestroyTexture(oldWindowTexture);
                windowStruct->sdlRenderer = newRenderer;
                windowStruct->sdlTexture  = NULL;
            }
        }
        windowStruct->sdlWindow = sdlWindow;
        windowStruct->mapState = Mapped;
        if (windowStruct->windowName != NULL) {
            free(windowStruct->windowName);
            windowStruct->windowName = NULL;
        }
        if (windowStruct->icon != NULL) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, windowStruct->icon);
        }
        parentWithSubstructureRedirect = enqueueMapEvent(display, window, None, True);
    } else {
        if (GET_WINDOW_STRUCT(GET_PARENT(window))->mapState == Mapped) {
            if (!mergeWindowDrawables(GET_PARENT(window), window)) {
                fprintf(stderr, "Failed to merge the window renderer in %s: %s\n", __func__, SDL_GetError());
                return;
            }
            parentWithSubstructureRedirect = enqueueMapEvent(display, window, None, True);
            WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
            windowStruct->mapState = Mapped;
        } else {
            if (!mergeWindowDrawables(GET_PARENT(window), window)) {
                fprintf(stderr, "Failed to merge the window renderer in %s: %s\n", __func__, SDL_GetError());
                return;
            }
            GET_WINDOW_STRUCT(window)->mapState = MapRequested;
            return;
        }
    }
    if (parentWithSubstructureRedirect == None) {
        parentWithSubstructureRedirect = getParentWithEventBit(window, SubstructureRedirectMask);
    }
    mapRequestedChildren(display, window, parentWithSubstructureRedirect);
    printWindowHierarchy();
}

void XUnmapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XUnmapWindow.html
    TYPE_CHECK(window, WINDOW, XCB_UNMAP_WINDOW, display, );
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_DestroyWindow(windowStruct->sdlWindow);
        windowStruct->sdlWindow = NULL;
        if (windowStruct->sdlRenderer != NULL) {
            SDL_DestroyRenderer(windowStruct->sdlRenderer);
            windowStruct->sdlRenderer = NULL;
        }
    }
    // TODO: Expose events
    windowStruct->mapState = UnMapped;
}

Status XWithdrawWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XWithdrawWindow.html
    TYPE_CHECK(window, WINDOW, XCB_UNMAP_WINDOW, display, 1);
    XUnmapWindow(display, window);
    return 1;
}

void XStoreName(Display* display, Window window, char* window_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XStoreName.html
    TYPE_CHECK(window, WINDOW, XCB_STORE_NAME, display, );
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_SetWindowTitle(GET_WINDOW_STRUCT(window)->sdlWindow, window_name);
    } else {
        char* windowName = malloc(sizeof(char) * (strlen(window_name) + 1));
        if (windowName == NULL) {
            handleError(0, display, window, 0, BadAlloc, XCB_STORE_NAME, 0);
            return;
        }
        strcpy(windowName, window_name);
        GET_WINDOW_STRUCT(window)->windowName = windowName;
    }
}

void XSetIconName(Display* display, Window window, char* icon_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetIconName.html
    // There is not really anything to do here
}

void XMoveWindow(Display* display, Window window, int x, int y) {
    // https://tronche.com/gui/x/xlib/window/XMoveWindow.html
    TYPE_CHECK(window, WINDOW, XCB_MOVE_WINDOW, display, );
    if (window != SCREEN_WINDOW) {
        WindowStruct *windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowPosition(windowStruct->sdlWindow, x, y);
        }
        windowStruct->x = x;
        windowStruct->y = y;
    }
}

void XResizeWindow(Display* display, Window window, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XResizeWindow.html
    TYPE_CHECK(window, WINDOW, XCB_RESIZE_WINDOW, display, );
    if (window != SCREEN_WINDOW) {
        WindowStruct *windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowSize(windowStruct->sdlWindow, (int) width, (int) height);
        }
        windowStruct->w = width;
        windowStruct->h = height;
        resizeWindowTexture(window);
    }
}

void XReparentWindow(Display* display, Window window, Window parent, int x, int y) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XReparentWindow.html
    TYPE_CHECK(window, WINDOW, XCB_REPARENT_WINDOW, display, );
    TYPE_CHECK(parent, WINDOW, XCB_REPARENT_WINDOW, display, );
    if (window == parent) {
        fprintf(stderr, "Invalid parameter: Can not add window to itself in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, XCB_REPARENT_WINDOW, 0);
        return;
    } else if (IS_INPUT_ONLY(parent) && !IS_INPUT_ONLY(window)) {
        fprintf(stderr, "Invalid parameter: Can not add InputOutput window to InputOnly window in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, XCB_REPARENT_WINDOW, 0);
        return;
    } else if (isParent(window, parent)) {
        fprintf(stderr, "Invalid parameter: Can not add window to one of it's childs in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, XCB_REPARENT_WINDOW, 0);
        return;
    }
    MapState mapState = GET_WINDOW_STRUCT(window)->mapState;
    XUnmapWindow(display, window);
    XMoveWindow(display, window, x, y);
    removeChildFromParent(window);
    if (!addChildToWindow(parent, window)) {
        fprintf(stderr, "Out of memory: Failed to reattach window in XReparentWindow!\n");
        return;
    }
    if (mapState == Mapped || mapState == MapRequested) {
        XMapWindow(display, window);
    }
    // TODO: ReparentNotify event
    XEvent event;
    event.type = ReparentNotify;
    event.xreparent.type = ReparentNotify;
    event.xreparent.serial = 0;
    event.xreparent.send_event = False;
    event.xreparent.display = display;
    event.xreparent.event = parent;
    event.xreparent.parent = parent;
    event.xreparent.window = window;
    event.xreparent.x = x;
    event.xreparent.y = y;
    event.xreparent.override_redirect = False;
    event.xany.serial = 0;
    event.xany.display = display;
    event.xany.send_event = False;
    event.xany.type = ReparentNotify;
    event.xany.window = parent;
    enqueueEvent(display, &event);
}

int indexInWindowList(Window* windowList, int numWindows, Window window) {
    int i;
    for (i = 0; i < numWindows; i++) {
        if (*windowList == window) {
            return i;
        }
        windowList++;
    }
    return -1;
}

Bool XTranslateCoordinates(Display* display, Window sourceWindow, Window destinationWindow,
                           int sourceX, int sourceY, int* destinationXReturn,
                           int* destinationYReturn, Window* childReturn) {
    // https://tronche.com/gui/x/xlib/window-information/XTranslateCoordinates.html
    TYPE_CHECK(sourceWindow, WINDOW, XCB_TRANSLATE_COORDINATES, display, False);
    TYPE_CHECK(destinationWindow, WINDOW, XCB_TRANSLATE_COORDINATES, display, False);
    int currX = sourceX;
    int currY = sourceY;
    int parentIndex;
    int i, x, y, width, height;
    int numDestParents;
    Window destParents[256];
    *destinationXReturn = 0;
    *destinationYReturn = 0;
    // Get all parents of destinationWindow
    if (destinationWindow == SCREEN_WINDOW) {
        destParents[0] = NULL;
        numDestParents = 0;
    } else {
        Window nextParent = GET_PARENT(destinationWindow);
        int numDestParents = 0;
        while (nextParent != SCREEN_WINDOW) {
            destParents[numDestParents++] = nextParent;
            if (nextParent == sourceWindow) {
                break; // sourceWindow is a parent of destinationWindow
            }
            nextParent = GET_PARENT(nextParent);
            if (numDestParents > 255) {
                fprintf(stderr, "Error: Unable to calculate common parent."\
                                "Number of parents exeeds 255 in XTranslateCoordinates!\n");
                return False;
            }
        }
        destParents[numDestParents] = NULL;
    }
    // Find the first common parent and translate sourceWindow's x and y to it's coordinate system
    while (sourceWindow != SCREEN_WINDOW) {
        parentIndex = indexInWindowList(&destParents[0], numDestParents, sourceWindow);
        if (parentIndex != -1) {
            break; // We got the first common parent
        }
        GET_WINDOW_POS(sourceWindow, x, y);
        currX += x;
        currY += y;
        sourceWindow = GET_PARENT(sourceWindow);
    }
    if (parentIndex == -1) {
        parentIndex = numDestParents; // SCREEN_WINDOW is the only common parent
    }
    // Translate x and y into destinationWindow's coordinate system
    while (--parentIndex > 0) {
        GET_WINDOW_POS(destParents[parentIndex], x, y);
        currX -= x;
        currY -= y;
    }
    GET_WINDOW_POS(destinationWindow, x, y);
    currX -= x;
    currY -= y;
    *destinationXReturn = currX;
    *destinationYReturn = currY;
    if (childReturn != NULL) {
        *childReturn = None;
        // Get the first child which contains x and y
        Window* childPointer = GET_CHILDREN(destinationWindow);
        int counter = -1;
        unsigned int childSpace = GET_WINDOW_STRUCT(destinationWindow)->childSpace;
        while (++counter < childSpace) {
            if (*childPointer != NULL) { continue; }
            GET_WINDOW_POS(*childPointer, x, y);
            GET_WINDOW_DIMS(*childPointer, width, height);
            if (x < currX && x + width > currX && y < currY && y + height > y) {
                (*childReturn)->type = WINDOW;
                (*childReturn)->dataPointer = *childPointer;
                break;
            }
            childPointer++;
        }
    }
    return True;
}

void XChangeProperty(Display* display, Window window, Atom property, Atom type, int format,
                     int mode, unsigned char* data, int numberOfElements) {
    // https://tronche.com/gui/x/xlib/window-information/XChangeProperty.html
    TYPE_CHECK(window, WINDOW, XCB_CHANGE_PROPERTY, display, );
    fprintf(stderr, "Changing window property %lu (%s).\n", property, (char*) XGetAtomName(display, property));
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, XCB_CHANGE_PROPERTY, 0);
        return;
    }
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    WindowProperty* windowProperty = findProperty(windowStruct->properties, windowStruct->propertyCount, property);
    unsigned char* combinedData = NULL;
    unsigned char* previousData = NULL;
    int previousDataSize = 0;
    int actualDataSize = numberOfElements * (format / 8);
    if (windowProperty != NULL) {
        previousDataSize = windowProperty->dataLength;
        previousData = windowProperty->data;
        if (windowProperty->type != type) {
            handleError(0, display, NULL, 0, BadMatch, XCB_CHANGE_PROPERTY, 0);
            return;
        }
    } else {
        windowProperty = malloc(sizeof(WindowProperty));
        if (windowProperty == NULL) {
            fprintf(stderr, "Out of memory: Failed to allocate space for new "
                    "windowProperty in XChangeProperty!\n");
            handleOutOfMemory(0, display, 0, XCB_CHANGE_PROPERTY, 0);
            return;
        }
        if (windowStruct->propertySize < windowStruct->propertyCount + 1) {
            unsigned int newSize;
            WindowProperty* newProperties = increasePropertySize(windowStruct->properties,
                                                                 windowStruct->propertySize, &newSize);
            if (newProperties == NULL) {
                fprintf(stderr, "Out of memory: Failed to increase property list size in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, XCB_CHANGE_PROPERTY, 0);
                free(windowProperty);
                return;
            }
            windowStruct->properties = newProperties;
            windowStruct->propertySize = newSize;
        }
        WindowProperty* propertyPointer = windowStruct->properties + windowStruct->propertyCount;
        propertyPointer = windowProperty;
        windowStruct->propertyCount++;
    }
    switch (mode) {
        case PropModeAppend:
        case PropModePrepend:
            combinedData = malloc(sizeof(unsigned char) * (previousDataSize + actualDataSize));
            if (combinedData == NULL) {
                fprintf(stderr, "Out of memory: Failed to allocate space for compined data "
                                "in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, XCB_CHANGE_PROPERTY, 0);
                if (previousData == NULL) { // We created the new property just above
                    free(windowStruct);
                    windowStruct->propertyCount--;
                }
                break;
            }
            if (mode == PropModeAppend) {
                if (previousData != NULL) {
                    memcpy(combinedData, previousData, sizeof(unsigned char) * previousDataSize);
                }
                memcpy(combinedData + previousDataSize, data,
                       sizeof(unsigned char) * actualDataSize);
            } else {
                memcpy(combinedData, data, sizeof(unsigned char) * actualDataSize);
                if (previousData != NULL) {
                    memcpy(combinedData + actualDataSize, previousData,
                           sizeof(unsigned char) * previousDataSize);
                }
            }
            windowProperty->data = combinedData;
            windowProperty->dataLength = actualDataSize + previousDataSize;
            break;
        case PropModeReplace:
            windowProperty->data = data;
            windowProperty->dataLength = actualDataSize;
            break;
        default:
            fprintf(stderr, "Bad parameter: Got unknown mode %d in XChangeProperty!\n", mode);
            handleError(0, display, NULL, 0, BadMatch, XCB_CHANGE_PROPERTY, 0);
            return;
    }
    windowProperty->property = property;
    windowProperty->type = type;
    windowProperty->dataFormat = format;
    if (previousData != NULL) {
        for (previousDataSize--; previousDataSize >= 0 ; previousDataSize--) {
            free(&previousData[previousDataSize]);
        }
    }
    if (property == _NET_WM_ICON) {
        // Find the icon with the highest resolution
        unsigned long* pixelData = (unsigned long*) data;
        unsigned long* icons[20];
        int i = 0, bestIcon = 0;
        unsigned long w;
        unsigned long h;
        do {
            icons[i++] = pixelData;
            w = pixelData[0];
            h = pixelData[1];
            if (w > icons[bestIcon][0] || h > icons[bestIcon][1]) {
                bestIcon = i;
            }
            pixelData += 2 + w * h;
        } while (i < 20 && pixelData < ((unsigned long*) data) + numberOfElements);
        w = icons[bestIcon][0];
        h = icons[bestIcon][1];
        SDL_Surface* icon = SDL_CreateRGBSurfaceFrom(&icons[bestIcon][2], (int) w, (int) h, 32, w * (32 / 8),
                                                     0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        windowStruct->icon = icon;
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, icon);
        }
    }
}

void XDeleteProperty(Display* display, Window window, Atom property) {
    // https://tronche.com/gui/x/xlib/window-information/XDeleteProperty.html
    TYPE_CHECK(window, WINDOW, XCB_DELETE_PROPERTY, display, );
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, XCB_DELETE_PROPERTY, 0);
        return;
    }
    if (property == _NET_WM_ICON) {
        windowStruct->icon = NULL;
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, NULL);
        }
    }
    WindowProperty* windowProperty = findProperty(windowStruct->properties,
                                                  windowStruct->propertyCount, property);
    if (windowProperty != NULL) {
        if (windowProperty->data != NULL) {
            for (windowProperty->dataLength--; windowProperty->dataLength >= 0 ; windowProperty->dataLength--) {
                free(&windowProperty->data[windowProperty->dataLength]);
            }
        }
        free(windowProperty);
        windowStruct->propertyCount--;
        // TODO: PropertyNotify event
    }
}

int XGetWindowProperty(Display* display, Window window, Atom property, long long_offset,
                       long long_length, Bool delete, Atom req_type, Atom *actual_type_return,
                       int* actual_format_return, unsigned long* numberOfItems_return,
                       unsigned long* bytes_after_return, unsigned char** prop_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetWindowProperty.html
    TYPE_CHECK(window, WINDOW, XCB_GET_PROPERTY, display, BadWindow);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, XCB_GET_PROPERTY, 0);
        return BadAtom;
    }
    WindowProperty* windowProperty = findProperty(windowStruct->properties,
                                                  windowStruct->propertyCount, property);
    unsigned char* pointer;
    if (windowProperty != NULL) {
        *actual_type_return = windowProperty->type;
        *actual_format_return = windowProperty->dataFormat;
        if (req_type == AnyPropertyType || req_type == windowProperty->type) {
            if (long_offset < 0) {
                fprintf(stderr, "Bad argument: long_offset is not positive "\
                        "in XGetWindowProperty!\n");
                handleError(0, display, NULL, 0, BadValue , XCB_GET_PROPERTY, 0);
                return BadValue;
            }
            int length = long_offset * 4 + long_length * 4 < windowProperty->dataLength ?
                         long_length * 4 : long_offset * 4 - windowProperty->dataLength;
            *prop_return = malloc(sizeof(unsigned char) * (length + 1));
            if (*prop_return == NULL) {
                fprintf(stderr, "Out of memory: Failed to allocate space for the retrun value "\
                        "in XGetWindowProperty!\n");
                handleOutOfMemory(0, display, 0, XCB_GET_PROPERTY, 0);
                return BadAlloc;
            }
            memcpy(*prop_return, windowProperty->data + long_offset * 4, length);
            *(*prop_return + length) = 0;
            *numberOfItems_return = length / windowProperty->dataFormat;
            *bytes_after_return = windowProperty->dataLength - (long_offset * 4 + length);
            if (delete && *bytes_after_return == 0) {
                XDeleteProperty(display, window, property);
            }
        } else {
            *bytes_after_return = (unsigned long) windowProperty->dataLength;
        }
    } else {
        *actual_type_return = None;
        *actual_format_return = 0;
        *bytes_after_return = 0;
    }
    return Success;
}

void XRaiseWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XRaiseWindow.html
    TYPE_CHECK(window, WINDOW, XCB_RAISE_WINDOW, display, );
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_RaiseWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
}

Status XGetWindowAttributes(Display* display, Window window,
                            XWindowAttributes* window_attributes_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
    TYPE_CHECK(window, WINDOW, XCB_GET_WINDOW_ATTRIBUTES, display, 0);
    window_attributes_return->root = SCREEN_WINDOW;
    window_attributes_return->visual = GET_VISUAL(window);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_Window* sdlWindow = GET_WINDOW_STRUCT(window)->sdlWindow;
        SDL_GetWindowPosition(sdlWindow, &window_attributes_return->x,
                              &window_attributes_return->y);
        SDL_GetWindowSize(sdlWindow, &window_attributes_return->width,
                          &window_attributes_return->height);
        Uint32 flags = SDL_GetWindowFlags(sdlWindow);
        if (HAS_VALUE(flags, SDL_WINDOW_MINIMIZED)) {
            window_attributes_return->map_state = IsUnviewable;
        } else if (HAS_VALUE(flags, SDL_WINDOW_HIDDEN)) {
            window_attributes_return->map_state = IsUnmapped;
        } else {
            window_attributes_return->map_state = IsViewable;
        }
    } else {
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        window_attributes_return->x      = windowStruct->x;
        window_attributes_return->y      = windowStruct->y;
        window_attributes_return->width  = windowStruct->w;
        window_attributes_return->height = windowStruct->h;
        // TODO: IsUnviewable?
        if (windowStruct->parent == NULL) {
            window_attributes_return->map_state = IsUnmapped;
        } else {
            window_attributes_return->map_state = IsViewable;
        }
    }
    // TODO: Not hardcode
    window_attributes_return->depth = 32;
    window_attributes_return->colormap = 0;
    return 1;
}

void XSetWindowBackground(Display* display, Window window, unsigned long background_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackground.html
    if (window != SCREEN_WINDOW) {
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background of an InputOnly "
                    "window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, XCB_CHANGE_WINDOW_ATTRIBUTES, 0);
            return;
        }
        GET_WINDOW_STRUCT(window)->backgroundColor = background_pixel;
    }
}

void XSetWindowBackgroundPixmap(Display* display, Window window, Pixmap background_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackgroundPixmap.html
    if (window != SCREEN_WINDOW) {
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background Pixmap of an "
                    "InputOnly window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, XCB_CHANGE_WINDOW_ATTRIBUTES, 0);
            return;
        }
        Pixmap previous = windowStruct->backgroundPixmap;
        if (background_pixmap == (Pixmap) ParentRelative) {
            windowStruct->backgroundPixmap = GET_PARENT(window) == NULL ? NULL :
                                             GET_WINDOW_STRUCT(GET_PARENT(window))->backgroundPixmap;
        } else {
            TYPE_CHECK(background_pixmap, PIXMAP, XCB_CHANGE_WINDOW_ATTRIBUTES, display, );
            windowStruct->backgroundPixmap = background_pixmap;
        }
        if (previous != NULL && previous != None) {
            XFreePixmap(display, previous);
        }
    }
}

void XSetWindowBorder(Display* display, Window window, unsigned long border_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorder.html
    //TODO: has no effect
}

void XSetWindowBorderPixmap(Display* display, Window window, Pixmap border_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderPixmap.html
    // TODO: has no effect
}

void XSetWindowColormap(Display* display, Window window, Colormap colormap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowColormap.html
    if (window != SCREEN_WINDOW) {
        GET_WINDOW_STRUCT(window)->colormap = colormap;
    }
}

void XChangeWindowAttributes(Display* display, Window window, unsigned long valueMask,
                             XSetWindowAttributes *attributes) {
    // https://tronche.com/gui/x/xlib/window/XChangeWindowAttributes.html
    TYPE_CHECK(window, WINDOW, XCB_CHANGE_WINDOW_ATTRIBUTES, display, );
    if (IS_ROOT(window) && HAS_VALUE(valueMask, CWCursor)) {
        XDefineCursor(display, window, attributes->cursor);
    }
    if (window != SCREEN_WINDOW) {
        if (HAS_VALUE(valueMask, CWBackPixmap)) {
            XSetWindowBackgroundPixmap(display, window, attributes->background_pixmap);
        }
        if (HAS_VALUE(valueMask, CWBackPixel)) {
            XSetWindowBackground(display, window, attributes->background_pixel);
        }
        if (HAS_VALUE(valueMask, CWColormap)) {
            XSetWindowColormap(display, window, attributes->colormap);
        }
        if (HAS_VALUE(valueMask, CWEventMask)) {
            fprintf(stderr, "Change window attributes event: %ld\n", attributes->event_mask & SubstructureRedirectMask);
            GET_WINDOW_STRUCT(window)->eventMask = attributes->event_mask;
            if (attributes->event_mask & KeyPressMask || attributes->event_mask & KeyReleaseMask) {
                // TODO: Implement real system here
                if (!SDL_IsTextInputActive()) {
                    SDL_StartTextInput();
                }
            }
        }
        // TODO: Interpret more values
    }
}

Window XRootWindow(Display* display, int screen_number) {
    // https://tronche.com/gui/x/xlib/display/display-macros.html#RootWindow
    return SCREEN_WINDOW;
}

void XMoveResizeWindow(Display* display, Window window, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XMoveResizeWindow.html
    XMoveWindow(display, window, x, y);
    XResizeWindow(display, window, width, height);
}

void XSetWindowBorderWidth(Display* display, Window window, unsigned int width) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderWidth.html
    // TODO: has no effect
}

Status XQueryTree(Display* display, Window window, Window* root_return, Window* parent_return,
                  Window** children_return, unsigned int* nchildren_return) {
    // https://tronche.com/gui/x/xlib/window-information/XQueryTree.html
    TYPE_CHECK(window, WINDOW, XCB_QUERY_TREE, display, 0);
    *root_return = SCREEN_WINDOW;
    *parent_return = GET_PARENT(window);
    *nchildren_return = 0;
    Window* children = GET_CHILDREN(window);
    int i, counter = 0;
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            *nchildren_return++;
        }
    }
    *children_return = malloc(sizeof(Window) * *nchildren_return);
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            *children_return[counter++] = children[i];
        }
    }
    return 1;
}
