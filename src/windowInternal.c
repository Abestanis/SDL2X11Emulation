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
    initArray(&windowStruct->children, 0);
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
    initArray(&windowStruct->properties, 0);
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
        for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->children.length; i++) {
            destroyWindow(display, children[i], False);
        }
        freeArray(&GET_WINDOW_STRUCT(SCREEN_WINDOW)->children);
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

void deleteWindowMapping(Window window) {
    WindowSdlIdMapper* mapper = mappingListStart;
    if (mapper == NULL) {
        return;
    } else if (mapper->window == window) {
        mappingListStart = mapper->next;
        free(mapper);
        return;
    }
    for (; mapper->next != NULL; mapper = mapper->next) {
        if (mapper->next->window == window) {
            WindowSdlIdMapper* nextMapper = mapper->next;
            mapper->next = nextMapper->next;
            free(nextMapper);
            return;
        }
    }
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

Window getContainingWindow(Window window, int x, int y) {
    int i, child_x, child_y, child_w, child_h;
    Window* children = GET_CHILDREN(window);
    for (i = GET_WINDOW_STRUCT(window)->children.length - 1; i >= 0 ; i--) {
        GET_WINDOW_POS(children[i], child_x, child_y);
        GET_WINDOW_DIMS(children[i], child_w, child_h);
        if (x >= child_x && x <= child_x + child_w && y >= child_y && y <= child_y + child_h) {
            return getContainingWindow(children[i], x - child_x, y - child_y);
        }
    }
    return window;
}

void removeChildFromParent(Window child) {
    if (child == SCREEN_WINDOW) { return; }
    Window parent = GET_PARENT(child);
    if (parent != NULL) {
        ssize_t childIndex = findInArray(&GET_WINDOW_STRUCT(parent)->children, child);
        if (childIndex != -1) {
            removeArray(&GET_WINDOW_STRUCT(parent)->children, (size_t) childIndex, True);
        }
    }
}

void destroyWindow(Display* display, Window window, Bool freeParentData) {
    size_t i;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->mapState == Mapped) {
        XUnmapWindow(display, window);
    }
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < windowStruct->children.length; i++) {
        destroyWindow(display, children[i], False);
    }
    freeArray(&windowStruct->children);
    XFreeColormap(display, GET_COLORMAP(window));
    for (i = 0; i < windowStruct->properties.length; i++) {
        free(windowStruct->properties.array[i]);
    }
    freeArray(&windowStruct->properties);
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
    deleteWindowMapping(window);
    postEvent(display, window, DestroyNotify);
    if (freeParentData) {
        removeChildFromParent(window);
    }
    free(windowStruct);
    free(window);
}

Bool addChildToWindow(Window parent, Window child) { // TODO: Check for duplicates?
    if (insertArray(&GET_WINDOW_STRUCT(parent)->children, child)) {
        GET_WINDOW_STRUCT(child)->parent = parent;
        return True;
    }
    return False;
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

WindowProperty* findProperty(Array* properties, Atom property, size_t* index) {
    size_t i;
    WindowProperty** windowProperties = (WindowProperty **) properties->array;
    for (i = 0; i < properties->length; i++) {
        if (windowProperties[i]->property == property) {
            if (index != NULL) *index = i;
            return windowProperties[i];
        }
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
    for (i = 0; i < GET_WINDOW_STRUCT(window)->children.length; i++) {
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
    Bool hasChanged = False;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!windowStruct->overrideRedirect && HAS_EVENT_MASK(GET_PARENT(window), SubstructureRedirectMask)) {
        return postEvent(display, window, ConfigureRequest, value_mask, values);
    }
    Bool isMappedTopLevelWindow = IS_MAPPED_TOP_LEVEL_WINDOW(window);
    int oldX, oldY, oldWidth, oldHeight;
    GET_WINDOW_POS(window, oldX, oldY);
    GET_WINDOW_DIMS(window, oldWidth, oldHeight);
    if (HAS_VALUE(value_mask, CWX) || HAS_VALUE(value_mask, CWY)) { 
        int x = oldX, y = oldY;
        if (HAS_VALUE(value_mask, CWX)) {
            x = values->x;
        }
        if (HAS_VALUE(value_mask, CWY)) {
            y = values->y;
        }
        if (isMappedTopLevelWindow) {
            SDL_SetWindowPosition(windowStruct->sdlWindow, x, y);
            SDL_GetWindowPosition(windowStruct->sdlWindow, &windowStruct->x, &windowStruct->y);
        } else {
            windowStruct->x = x;
            windowStruct->y = y;
        }
        if (oldX != windowStruct->x || oldY != windowStruct->y) {
            hasChanged = True;
        }
    }
    if (HAS_VALUE(value_mask, CWWidth) || HAS_VALUE(value_mask, CWHeight)) {
        int width = oldWidth, height = oldHeight;
        if (HAS_VALUE(value_mask, CWWidth)) {
            width = values->width;
            if (width <= 0) {
                handleError(0, display, NULL, 0, BadValue, 0);
                return False;
            }
        }
        if (HAS_VALUE(value_mask, CWHeight)) {
            height = values->height;
            if (height <= 0) {
                handleError(0, display, NULL, 0, BadValue, 0);
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
        if (oldWidth != windowStruct->w || oldHeight != windowStruct->h) {
            resizeWindowSurface(window); // TODO: Handle fail
            hasChanged = True;
        }
    }
    if (!hasChanged) return True;
    if (!postEvent(display, window, ConfigureNotify)) {
        return False;
    }
    if (windowStruct->mapState != UnMapped && (oldX != windowStruct->x || oldY != windowStruct->y
        || oldWidth != windowStruct->w || oldHeight != windowStruct->h)) {
        SDL_Rect exposedRect;  // TODO: Handle whe window shrinks or moves, update parent
        if (oldX != windowStruct->x || oldY != windowStruct->y) {
            exposedRect.x = 0;
            exposedRect.y = 0;
            exposedRect.w = windowStruct->w;
            exposedRect.h = windowStruct->h;
        } else {
            exposedRect.x = 0;
            exposedRect.y = 0;
            exposedRect.w = windowStruct->w;
            exposedRect.h = windowStruct->h;// TODO Calculate exposed rect
        }
        postExposeEvent(display, window, &exposedRect, 1);
    }
    return True;
    // TODO: Implement re-stacking: https://tronche.com/gui/x/xlib/window/configure.html#XWindowChanges
}
