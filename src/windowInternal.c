//
// Created by Sebastian on 08.10.2016.
//
#include "windowInternal.h"
#include "drawing.h"
#include "events.h"

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
    windowStruct->unmappedContent = NULL;
    windowStruct->sdlWindow = NULL;
    windowStruct->renderTarget = NULL;
    windowStruct->backgroundColor = backgroundColor;
    windowStruct->background = backgroundPixmap;
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
    windowStruct->eventMask = NoEventMask;
    windowStruct->overrideRedirect = False;
#ifdef DEBUG_WINDOWS
    windowStruct->debugId = ((unsigned long) rand() << 16) | rand();
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
        window->mapState = Mapped;
    }
    return True;
}

void destroyScreenWindow(Display* display) {
    if (SCREEN_WINDOW != NULL) {
        size_t i;
        Window* children = GET_CHILDREN(SCREEN_WINDOW);
        for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
            if (children[i] != NULL) {
                destroyWindow(display, children[i], False);
                children[i] = NULL;
            }
        }
        free(children);
        GPU_FreeTarget(GET_WINDOW_STRUCT(SCREEN_WINDOW)->renderTarget);
        SDL_DestroyWindow(GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlWindow);
        free(SCREEN_WINDOW->dataPointer);
        free(SCREEN_WINDOW);
        SCREEN_WINDOW = NULL;
    }
}

WindowSdlIdMapper* mappingListStart = NULL;

WindowSdlIdMapper* getWindowSdlIdMapperStructFromId(Uint32 sdlWindowId) {
    WindowSdlIdMapper* mapper;
    for (mapper = mappingListStart; mapper != NULL; mapper = mapper->next) {
        if (mapper->sdlWindowId == sdlWindowId) { return mapper; }
    }
    return NULL;
}

//TODO: Unregister window mapping

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
    size_t i;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->mapState == Mapped) {
        XUnmapWindow(display, window);
    }
    if (windowStruct->childSpace > 0) {
        Window* children = GET_CHILDREN(window);
        for (i = 0; i < windowStruct->childSpace; i++) {
            if (children[i] != NULL) {
                destroyWindow(display, children[i], False);
                children[i] = NULL;
            }
        }
        free(children);
        windowStruct->childSpace = 0;
        windowStruct->children = NULL;
    }
    XFreeColormap(display, GET_COLORMAP(window));
    if (windowStruct->propertySize > 0) {
        WindowProperty* propertyPointer = windowStruct->properties;
        free(propertyPointer);
    }
    if (windowStruct->background != NULL && windowStruct->background != None) {
        XFreePixmap(display, windowStruct->background);
    }
    if (windowStruct->windowName != NULL) {
        free(windowStruct->windowName);
    }
    if (windowStruct->icon != NULL) {
        SDL_FreeSurface(windowStruct->icon);
    }
    if (windowStruct->renderTarget != NULL) {
        GPU_FreeTarget(windowStruct->renderTarget);
    }
    if (windowStruct->unmappedContent != NULL) {
        GPU_FreeImage(windowStruct->unmappedContent);
    }
    if (windowStruct->sdlWindow != NULL) {
        SDL_DestroyWindow(windowStruct->sdlWindow);
    }
    postEvent(display, window, DestroyNotify);
    if (freeParentData) {
        removeChildFromParent(window);
    }
    free(windowStruct);
    free(window);
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

Bool resizeWindowSurface(Window window) {
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    GPU_Image* oldContent = windowStruct->unmappedContent;
    if (oldContent != NULL) {
        GPU_Image* newContent = GPU_CreateImage((Uint16) windowStruct->w, (Uint16) windowStruct->h, oldContent->format);
        if (newContent == NULL) {
            fprintf(stderr, "Failed to resize the window surface: Failed to create new window surface!\n");
            return FALSE;
        }
        GPU_Target* newTarget = GPU_LoadTarget(newContent);
        if (newTarget == NULL) {
            GPU_FreeImage(newContent);
            fprintf(stderr, "Failed to resize the window surface: Failed to create render target from new window surface!\n");
            return FALSE;
        }
        if (windowStruct->renderTarget != NULL) GPU_Flip(windowStruct->renderTarget);
        fprintf(stderr, "Resizing surface of window %p\n", window);
        fprintf(stderr, "BLITTING in %s\n", __func__);
        GPU_Blit(oldContent, NULL, newTarget, oldContent->w / 2, oldContent->h / 2);
        if (windowStruct->renderTarget != NULL) GPU_FreeTarget(windowStruct->renderTarget);
        GPU_FreeImage(oldContent);
        windowStruct->unmappedContent = newContent;
        windowStruct->renderTarget = newTarget;
    }
    return TRUE;
}

Bool mergeWindowDrawables(Window parent, Window child) {
    WindowStruct* childWindowStruct = GET_WINDOW_STRUCT(child);
    if (childWindowStruct->unmappedContent == NULL) { return True; }
    fprintf(stderr, "getWindowRenderTarget of window %p in %s.\n", parent, __func__);
    GPU_Target* parentTarget = getWindowRenderTarget(parent);
    if (parentTarget == NULL) return FALSE;
    if (childWindowStruct->renderTarget != NULL) {
        GPU_Flip(childWindowStruct->renderTarget);
    }
    fprintf(stderr, "BLITTING in %s\b", __func__);
    GPU_Blit(childWindowStruct->unmappedContent, NULL, parentTarget,
             childWindowStruct->x + childWindowStruct->w / 2, childWindowStruct->y + childWindowStruct->h / 2);
    if (childWindowStruct->renderTarget != NULL) {
        GPU_FreeTarget(childWindowStruct->renderTarget);
        childWindowStruct->renderTarget = NULL;
    }
    GPU_FreeImage(childWindowStruct->unmappedContent);
    childWindowStruct->unmappedContent = NULL;
    return True;
}

void mapRequestedChildren(Display* display, Window window) {
    Window* children = GET_CHILDREN(window);
    size_t i;
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->mapState == MapRequested) {
            if (!mergeWindowDrawables(window, children[i])) {
                fprintf(stderr, "Failed to merge the window drawables in %s\n", __func__);
                return;
            }
            GET_WINDOW_STRUCT(children[i])->mapState = Mapped;
            postEvent(display, children[i], MapNotify);
            mapRequestedChildren(display, children[i]);
        }
    }
}

Bool configureWindow(Display* display, Window window, unsigned long value_mask, XWindowChanges* values) {
    if (window == SCREEN_WINDOW) return True;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!windowStruct->overrideRedirect && HAS_EVENT_MASK(GET_PARENT(window), SubstructureRedirectMask)) {
        return postEvent(display, window, ConfigureRequest, value_mask, values);
    }
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
        // TODO: Generate expose events
    }
    if (HAS_VALUE(value_mask, CWWidth) || HAS_VALUE(value_mask, CWHeight)) {
        int width, height;
        GET_WINDOW_DIMS(window, width, height);
        int oldWidth = width, oldHeight = height;
        if (HAS_VALUE(value_mask, CWWidth)) {
            width = values->width;
            if (width <= 0) {
                handleError(BadValue, display, NULL, 0, 0, 0);
                return False;
            }
        }
        if (HAS_VALUE(value_mask, CWHeight)) {
            height = values->height;
            if (height <= 0) {
                handleError(BadValue, display, NULL, 0, 0, 0);
                return False;
            }
        }
        printWindowsHierarchy();
        fprintf(stderr, "Resizing window %p to (%ux%u)\n", window, width, height);
        if (isMappedTopLevelWindow) {
            SDL_SetWindowSize(windowStruct->sdlWindow, width, height);
            int wOut, hOut;
            SDL_GetWindowSize(windowStruct->sdlWindow, &wOut, &hOut);
            windowStruct->w = (unsigned int) wOut;
            windowStruct->h = (unsigned int) hOut;
        } else {
            windowStruct->w = (unsigned int) width;
            windowStruct->h = (unsigned int) height;
        }
        resizeWindowSurface(window); // TODO: Handle fail
        if (oldWidth < width || oldHeight < height) {
            SDL_Rect exposedRect = { 0, 0, width, height }; // TODO: Calculate exposed rect
            postExposeEvent(display, window, exposedRect);
        }
    }
    return postEvent(display, window, ConfigureNotify);
    // TODO: Implement re-stacking: https://tronche.com/gui/x/xlib/window/configure.html#XWindowChanges
}
