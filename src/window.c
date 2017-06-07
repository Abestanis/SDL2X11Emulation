#include <X11/Xlib.h>
#include "window.h"
#include "errors.h"
#include "drawing.h"
#include "atoms.h"
#include "events.h"
#include "display.h"

// TODO: Cover cases where top-level window is re-parented and window is converted to top-level window

int XDestroyWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XDestroyWindow.html
    SET_X_SERVER_REQUEST(display, X_DestroyWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (window == SCREEN_WINDOW) return 0;
    destroyWindow(display, window, True);
    return 1;
}

Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width,
                     unsigned int height, unsigned int border_width, int depth, unsigned int clazz,
                     Visual* visual, unsigned long valueMask, XSetWindowAttributes* attributes) {
    // https://tronche.com/gui/x/xlib/window/XCreateWindow.html
    SET_X_SERVER_REQUEST(display, X_CreateWindow);
    TYPE_CHECK(parent, WINDOW, display, None);
    Bool inputOnly = (clazz == InputOnly || (clazz == CopyFromParent && IS_INPUT_ONLY(parent)));
    if (inputOnly && border_width != 0) {
        fprintf(stderr, "Bad argument: Given class is InputOnly but border_with is not 0 in XCreateWindow!\n");
        handleError(0, display, None, 0, BadMatch, 0);
        return None;
    }
    Window windowID = ALLOC_XID();
    if (windowID == None) {
        fprintf(stderr, "Out of memory: Could not allocate the window id in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        return None;
    }
    WindowStruct* windowStruct = malloc(sizeof(WindowStruct));
    if (windowStruct == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window struct in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        FREE_XID(windowID);
        return None;
    }
    SET_XID_TYPE(windowID, WINDOW);
    SET_XID_VALUE(windowID, windowStruct);
    initWindowStruct(windowStruct, x, y, width, height, visual, None, inputOnly, 0, None);
    windowStruct->depth = depth;
    windowStruct->borderWidth = border_width;
    if (!addChildToWindow(parent, windowID)) {
        fprintf(stderr, "Out of memory: Could not increase size of parent's child list in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        free(windowStruct);
        FREE_XID(windowID);
        return None;
    }
    int visualClass = visual->CLASS_ATTRIBUTE;
    windowStruct->colormap = (Colormap) XCreateColormap(display, windowID, visual,
                                                         visualClass == StaticGray ||
                                                         visualClass == StaticColor ||
                                                         visualClass == TrueColor ?
                                                         AllocAll : AllocNone);
    SET_X_SERVER_REQUEST(display, X_CreateWindow);
    if (windowStruct->colormap == None) {
        fprintf(stderr, "Out of memory: Could not allocate the window colormap in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        free(windowStruct);
        FREE_XID(windowID);
        return None;
    }
    // FIXME: Warning: Colormap is not initialized!
    // Set up the window ahead of time for event processing, so we can send the CreateNotify event
    if (HAS_VALUE(valueMask, CWEventMask)) windowStruct->eventMask = attributes->event_mask;
    postEvent(display, windowID, CreateNotify); 
    if (valueMask != 0) {
        XChangeWindowAttributes(display, windowID, valueMask, attributes);
    }
    return windowID;
}

int XConfigureWindow(Display* display, Window window, unsigned int value_mask,
                      XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/window/XConfigureWindow.html
    SET_X_SERVER_REQUEST(display, X_ConfigureWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    return configureWindow(display, window, value_mask, values);
}

Status XReconfigureWMWindow(Display* display, Window window, int screen_number,
                            unsigned int value_mask, XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XReconfigureWMWindow.html
    XConfigureWindow(display, window, value_mask, values);
    return 1;
}

Status XIconifyWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XIconifyWindow.html
    TYPE_CHECK(window, WINDOW, display, 0);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_MinimizeWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
    return 1;
}

Status XSetWMColormapWindows(Display* display, Window window, Window* colormap_windows, int count) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetWMColormapWindows.html
    // TODO: Should we do sth. with the information?
    TYPE_CHECK(window, WINDOW, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    windowStruct->colormapWindowsCount = count;
    windowStruct->colormapWindows = colormap_windows;
    return 1;
}

Status XGetWMColormapWindows(Display* display, Window window, Window** colormap_windows_return, int* count_return) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XGetWMColormapWindows.html
    TYPE_CHECK(window, WINDOW, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->colormapWindowsCount > -1) {
        *count_return = windowStruct->colormapWindowsCount;
        *colormap_windows_return = windowStruct->colormapWindows;
        return 1;
    }
    return 0;
}

int XSetTransientForHint(Display* display, Window window, Window prop_window) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetTransientForHint.html
    // TODO: Ignored for now
    return 1;
}

int XMapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XMapWindow.html
    SET_X_SERVER_REQUEST(display, X_MapWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (GET_WINDOW_STRUCT(window)->mapState == Mapped || GET_WINDOW_STRUCT(window)->mapState == MapRequested) { return 1; }
    if (!GET_WINDOW_STRUCT(window)->overrideRedirect && HAS_EVENT_MASK(GET_PARENT(window), SubstructureRedirectMask)) {
        postEvent(display, window, MapRequest);
        return 1;
    }
    if (IS_TOP_LEVEL(window)) {
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) { return 1; }
        fprintf(stderr, "Mapping Window %lu\n", window);
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (windowStruct->borderWidth == 0) {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        SDL_Window* sdlWindow = SDL_CreateWindow(windowStruct->windowName,
                                                 windowStruct->x, windowStruct->y,
                                                 windowStruct->w, windowStruct->h, flags);
        if (sdlWindow == NULL) {
            fprintf(stderr, "SDL_CreateWindow failed in XMapWindow: %s\n", SDL_GetError());
            handleError(0, display, None, 0, BadMatch, 0);
            return 0;
        }
        registerWindowMapping(window, SDL_GetWindowID(sdlWindow));
        GPU_Target* renderTarget = GPU_CreateTargetFromWindow(SDL_GetWindowID(sdlWindow));
        if (renderTarget == NULL) {
            fprintf(stderr, "GPU_CreateTargetFromWindow failed in XMapWindow: %s\n", GPU_PopErrorCode().details);
            handleError(0, display, None, 0, BadMatch, 0);
            return 0;
        }
        if (windowStruct->unmappedContent != NULL) {
            if (windowStruct->renderTarget != NULL) {
                GPU_Flip(windowStruct->renderTarget);
            }
            fprintf(stderr, "BLITTING in %s\n", __func__);
            int x, y;
            GET_WINDOW_POS(window, x, y);
            GPU_Blit(windowStruct->unmappedContent, NULL, renderTarget,
                     x + windowStruct->w / 2, y + windowStruct->h / 2);
        }
        if (windowStruct->renderTarget != NULL) {
            GPU_FreeTarget(windowStruct->renderTarget);
        }
        if (windowStruct->unmappedContent != NULL) {
            GPU_FreeImage(windowStruct->unmappedContent);
            windowStruct->unmappedContent = NULL;
        }
        windowStruct->renderTarget = renderTarget;
        windowStruct->sdlWindow = sdlWindow;
        windowStruct->mapState = Mapped;
        if (windowStruct->windowName != NULL) {
            free(windowStruct->windowName);
            windowStruct->windowName = NULL;
        }
        if (windowStruct->icon != NULL) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, windowStruct->icon);
        }
    } else { /* Mapping a window that is not a top level window  */
        Window parent = GET_PARENT(window);
        if (GET_WINDOW_STRUCT(parent)->mapState == Mapped) {
            if (!mergeWindowDrawables(parent, window)) {
                fprintf(stderr, "Failed to merge the window drawables in %s\n", __func__);
                return 0;
            }
            GET_WINDOW_STRUCT(window)->mapState = Mapped;
        } else { /* Parent not mapped */
            // mapRequestedChildren will do all the work
            // TODO: Have a look at this: https://tronche.com/gui/x/xlib/window/map.html
            GET_WINDOW_STRUCT(window)->mapState = MapRequested;
            return 0;
        }
    }
    postEvent(display, window, MapNotify);
    mapRequestedChildren(display, window);
    #ifdef DEBUG_WINDOWS
    printWindowsHierarchy();
    #endif
    return 1;
}

int XUnmapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XUnmapWindow.html
    SET_X_SERVER_REQUEST(display, X_UnmapWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (window == SCREEN_WINDOW) {
        handleError(0, display, window, 0, BadWindow, 0);
        return 0;
    }
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->mapState == UnMapped) return 1;
    if (windowStruct->renderTarget != NULL) {
        GPU_FreeTarget(windowStruct->renderTarget);
        windowStruct->renderTarget = NULL;
    }
    windowStruct->mapState = UnMapped;
    if (windowStruct->sdlWindow != NULL) {
        SDL_Window* sdlWindow = windowStruct->sdlWindow;
        windowStruct->sdlWindow = NULL;
        SDL_DestroyWindow(sdlWindow);
    } else if (GET_WINDOW_STRUCT(GET_PARENT(window))->mapState != UnMapped) {
        postEvent(display, window, UnmapNotify, False);
        SDL_Rect exposeRect = {windowStruct->x, windowStruct->y, windowStruct->w, windowStruct->h};
        postExposeEvent(display, GET_PARENT(window), &exposeRect, 1);
    }
    // TODO: Change subwindow state to MapRequested?
    return 1;
}

Status XWithdrawWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XWithdrawWindow.html
    TYPE_CHECK(window, WINDOW, display, 0);
    XUnmapWindow(display, window); // TODO
    return 1;
}

int XStoreName(Display* display, Window window, _Xconst char* window_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XStoreName.html
    TYPE_CHECK(window, WINDOW, display, 0);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_SetWindowTitle(GET_WINDOW_STRUCT(window)->sdlWindow, window_name);
    } else {
        char* windowName = malloc(sizeof(char) * (strlen(window_name) + 1));
        if (windowName == NULL) {
            handleError(0, display, window, 0, BadAlloc, 0);
            return 0;
        }
        strcpy(windowName, window_name);
        GET_WINDOW_STRUCT(window)->windowName = windowName;
    }
    return 1;
}

int XSetIconName(Display* display, Window window, _Xconst char* icon_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetIconName.html
//    SET_X_SERVER_REQUEST(display, XCB_);
    // There is not really anything to do here // TODO: Check if this is true
    return 1;
}

int XMoveWindow(Display* display, Window window, int x, int y) {
    // https://tronche.com/gui/x/xlib/window/XMoveWindow.html
    SET_X_SERVER_REQUEST(display, X_ConfigureWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    return configureWindow(display, window, CWX | CWY, &changes);
}

int XResizeWindow(Display* display, Window window, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XResizeWindow.html
    SET_X_SERVER_REQUEST(display, X_ConfigureWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    XWindowChanges changes;
    changes.width = width;
    changes.height = height;
    return configureWindow(display, window, CWWidth | CWHeight, &changes);
}

int XReparentWindow(Display* display, Window window, Window parent, int x, int y) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XReparentWindow.html
    SET_X_SERVER_REQUEST(display, X_ReparentWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    TYPE_CHECK(parent, WINDOW, display, 0);
    if (window == parent) {
        fprintf(stderr, "Invalid parameter: Can not add window to itself in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return 0;
    } else if (IS_INPUT_ONLY(parent) && !IS_INPUT_ONLY(window)) {
        fprintf(stderr, "Invalid parameter: Can not add InputOutput window to InputOnly window in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return 0;
    } else if (isParent(window, parent)) {
        fprintf(stderr, "Invalid parameter: Can not add window to one of it's childs in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return 0;
    }
    MapState mapState = GET_WINDOW_STRUCT(window)->mapState;
    XUnmapWindow(display, window);
    Window oldParent = GET_PARENT(window);
    removeChildFromParent(window);
    if (!addChildToWindow(parent, window)) {
        fprintf(stderr, "Out of memory: Failed to reattach window in XReparentWindow!\n");
        return 0;
    }
    XMoveWindow(display, window, x, y); // TODO: Do this without generating events
    postEvent(display, window, ReparentNotify, oldParent);
    if (mapState != UnMapped) {
        XMapWindow(display, window);
    }
    return 1;
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
    SET_X_SERVER_REQUEST(display, X_TranslateCoords);
    TYPE_CHECK(sourceWindow, WINDOW, display, False);
    TYPE_CHECK(destinationWindow, WINDOW, display, False);
    int currX = sourceX;
    int currY = sourceY;
    int parentIndex = -1;
    int x, y, width, height;
    int numDestParents;
    Window destParents[256];
    *destinationXReturn = 0;
    *destinationYReturn = 0;
    // Get all parents of destinationWindow
    if (destinationWindow == SCREEN_WINDOW) {
        destParents[0] = None;
        numDestParents = 0;
    } else {
        Window nextParent = GET_PARENT(destinationWindow);
        numDestParents = 0;
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
        destParents[numDestParents] = None;
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
        Window* children = GET_CHILDREN(destinationWindow);
        size_t i;
        for (i = 0; i < GET_WINDOW_STRUCT(destinationWindow)->children.length; i++) {
            GET_WINDOW_POS(children[i], x, y);
            GET_WINDOW_DIMS(children[i], width, height);
            if (x < currX && x + width > currX && y < currY && y + height > y) {
                *childReturn = children[i];
                break;
            }
        }
    }
    return True;
}

int XChangeProperty(Display* display, Window window, Atom property, Atom type, int format,
                     int mode, _Xconst unsigned char* data, int numberOfElements) {
    // https://tronche.com/gui/x/xlib/window-information/XChangeProperty.html
    SET_X_SERVER_REQUEST(display, X_ChangeProperty);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (numberOfElements < 0) {
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    if (format != 8 && format != 16 && format != 32) {
        handleError(0, display, None, 0, BadValue, 0);
        return 0;
    }
    fprintf(stderr, "Changing window property %lu (%s).\n", property, getAtomName(property));
    if (!isValidAtom(property)) {
        handleError(0, display, property, 0, BadAtom, 0);
        return 0;
    }
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    WindowProperty* windowProperty = findProperty(&windowStruct->properties, property, NULL);
    unsigned char* combinedData = NULL;
    unsigned char* previousData = NULL;
    unsigned int previousDataLength = 0;
    Bool propertyIsNew = windowProperty == NULL;
    size_t dataTypeSize = format == 8 ? sizeof(char) : (format == 16 ? sizeof(short) : sizeof(long));
    if (!propertyIsNew) {
        previousDataLength = windowProperty->dataLength;
        previousData = windowProperty->data;
        if (windowProperty->type != type) {
            handleError(0, display, type, 0, BadMatch, 0);
            return 0;
        }
    } else {
        windowProperty = malloc(sizeof(WindowProperty));
        if (windowProperty == NULL) {
            handleOutOfMemory(0, display, 0, 0);
            return 0;
        }
        if (!insertArray(&windowStruct->properties, windowProperty)) {
            free(windowProperty);
            handleOutOfMemory(0, display, 0, 0);
            return 0;
        }
        windowProperty->dataFormat = format;
        windowProperty->data = NULL;
        windowProperty->dataLength = 0;
        windowProperty->property = property;
        windowProperty->type = type;
    }
    switch (mode) {
        case PropModeAppend:
        case PropModePrepend:
            if (format != windowProperty->dataFormat || type != windowProperty->type) {
                if (propertyIsNew) {
                    removeArray(&windowStruct->properties, windowStruct->properties.length - 1, False);
                    free(windowProperty);
                }
                handleError(0, display, None, 0, BadMatch, 0);
                return 0;
            }
            combinedData = malloc(dataTypeSize * (previousDataLength + numberOfElements));
            if (combinedData == NULL) {
                if (propertyIsNew) {
                    removeArray(&windowStruct->properties, windowStruct->properties.length - 1, False);
                    free(windowProperty);
                }
                fprintf(stderr, "Out of memory: Failed to allocate space for combined data "
                                "in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
                return 0;
            }
            if (mode == PropModeAppend) {
                if (previousData != NULL) {
                    memcpy(combinedData, previousData, dataTypeSize * previousDataLength);
                }
                memcpy(combinedData + dataTypeSize * previousDataLength, data, dataTypeSize * numberOfElements);
            } else {
                memcpy(combinedData, data, dataTypeSize * numberOfElements);
                if (previousData != NULL) {
                    memcpy(combinedData + dataTypeSize * numberOfElements, previousData, dataTypeSize * previousDataLength);
                }
            }
            windowProperty->data = combinedData;
            windowProperty->dataLength = numberOfElements + previousDataLength;
            break;
        case PropModeReplace:
            windowProperty->data = malloc(dataTypeSize * numberOfElements);
            if (windowProperty->data == NULL) {
                if (propertyIsNew) {
                    removeArray(&windowStruct->properties, windowStruct->properties.length - 1, False);
                    free(windowProperty);
                }
                fprintf(stderr, "Out of memory: Failed to allocate space for data "
                        "in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
                return 0;
            }
            memcpy(windowProperty->data, data, dataTypeSize * numberOfElements);
            windowProperty->dataLength = (unsigned int) numberOfElements;
            windowProperty->property = property;
            windowProperty->type = type;
            windowProperty->dataFormat = format;
            break;
        default:
            if (propertyIsNew) {
                removeArray(&windowStruct->properties, windowStruct->properties.length - 1, False);
                free(windowProperty);
            }
            fprintf(stderr, "Bad parameter: Got unknown mode %d in XChangeProperty!\n", mode);
            handleError(0, display, None, 0, BadMatch, 0);
            return 0;
    }
    if (previousData != NULL) {
//        free(previousData); // TODO: Fix crash here
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
        if (windowStruct->icon != NULL) {
            SDL_FreeSurface(windowStruct->icon);
        }
        windowStruct->icon = icon;
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, icon);
        }
    }
    return 1;
}

int XDeleteProperty(Display* display, Window window, Atom property) {
    // https://tronche.com/gui/x/xlib/window-information/XDeleteProperty.html
    SET_X_SERVER_REQUEST(display, X_DeleteProperty);
    TYPE_CHECK(window, WINDOW, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, property, 0, BadAtom, 0);
        return 0;
    }
    if (property == _NET_WM_ICON) {
        if (windowStruct->icon != NULL) {
            SDL_FreeSurface(windowStruct->icon);
            windowStruct->icon = NULL;
            if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
                SDL_SetWindowIcon(windowStruct->sdlWindow, NULL);
            }
        }
    }
    size_t index, i;
    WindowProperty* windowProperty = findProperty(&windowStruct->properties, property, &index);
    if (windowProperty != NULL) {
        if (windowProperty->data != NULL) {
            for (i = 0; i < windowProperty->dataLength; i++) {
                free(&windowProperty->data[i]);
            }
        }
        removeArray(&windowStruct->properties, index, False);
        // TODO: PropertyNotify event
    }
    return 1;
}

int XGetWindowProperty(Display* display, Window window, Atom property, long long_offset,
                       long long_length, Bool delete, Atom req_type, Atom *actual_type_return,
                       int* actual_format_return, unsigned long* numberOfItems_return,
                       unsigned long* bytes_after_return, unsigned char** prop_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetWindowProperty.html
    SET_X_SERVER_REQUEST(display, X_GetProperty);
    TYPE_CHECK(window, WINDOW, display, BadWindow);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, property, 0, BadAtom, 0);
        return BadAtom;
    }
    WindowProperty* windowProperty = findProperty(&windowStruct->properties, property, NULL);
    if (windowProperty != NULL) {
        *actual_type_return = windowProperty->type;
        *actual_format_return = windowProperty->dataFormat;
        size_t dataTypeSize = windowProperty->dataFormat == 8 ? sizeof(char) :
                              (windowProperty->dataFormat == 16 ? sizeof(short) : sizeof(long));
        if (req_type == AnyPropertyType || req_type == windowProperty->type) {
            if (long_offset < 0 || windowProperty->dataLength * dataTypeSize < 4 * long_offset) {
                handleError(0, display, None, 0, BadValue, 0);
                return BadValue;
            }
            size_t dataReturnSize = MIN(windowProperty->dataLength * dataTypeSize - 4 * long_offset, 4 * (size_t) long_length);
            *prop_return = malloc(sizeof(char) * (dataReturnSize + 1));
            if (*prop_return == NULL) {
                fprintf(stderr, "Out of memory: Failed to allocate space for the return value "\
                        "in XGetWindowProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
                return BadAlloc;
            }
            memcpy(*prop_return, windowProperty->data + long_offset * 4, dataReturnSize);
            (*prop_return)[dataReturnSize] = '\0';
            *numberOfItems_return = dataReturnSize / dataTypeSize;
            *bytes_after_return = windowProperty->dataLength * dataTypeSize - (long_offset * 4 + dataReturnSize);
            if (delete && *bytes_after_return == 0) {
                XDeleteProperty(display, window, property);
            }
        } else {
            *bytes_after_return = (unsigned long) windowProperty->dataLength * dataTypeSize;
            *numberOfItems_return = 0;
        }
    } else {
        *actual_type_return = None;
        *actual_format_return = 0;
        *bytes_after_return = 0;
        *numberOfItems_return = 0;
    }
    return Success;
}

int XRaiseWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XRaiseWindow.html
    SET_X_SERVER_REQUEST(display, X_ConfigureWindow);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_RaiseWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
    // TODO: Rearrange child in child list of parent.
    return 1;
}

Status XGetWindowAttributes(Display* display, Window window,
                            XWindowAttributes* window_attributes_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
    SET_X_SERVER_REQUEST(display, X_GetWindowAttributes);
    TYPE_CHECK(window, WINDOW, display, 0);
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
        if (windowStruct->parent == None) {
            window_attributes_return->map_state = IsUnmapped;
        } else {
            window_attributes_return->map_state = IsViewable;
        }
    }
    window_attributes_return->depth = SDL_SURFACE_DEPTH;
    window_attributes_return->colormap = GET_WINDOW_STRUCT(window)->colormap;
    return 1;
}

int XSetWindowBackground(Display* display, Window window, unsigned long background_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackground.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    if (window != SCREEN_WINDOW) {
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background of an InputOnly "
                    "window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, 0);
            return 0;
        }
        GET_WINDOW_STRUCT(window)->backgroundColor = background_pixel;
    }
    return 1;
}

int XSetWindowBackgroundPixmap(Display* display, Window window, Pixmap background_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackgroundPixmap.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    if (window != SCREEN_WINDOW) {
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background Pixmap of an "
                    "InputOnly window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, 0);
            return 0;
        }
        Pixmap previous = windowStruct->background;
        if (background_pixmap == (Pixmap) ParentRelative) {
            windowStruct->background = GET_PARENT(window) == None ? None :
                                             GET_WINDOW_STRUCT(GET_PARENT(window))->background;
        } else {
            TYPE_CHECK(background_pixmap, PIXMAP, display, 0);
            windowStruct->background = background_pixmap;
        }
        if (previous != None) {
            XFreePixmap(display, previous);
        }
    }
    return 1;
}

int XSetWindowBorder(Display* display, Window window, unsigned long border_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorder.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    //TODO: has no effect
    return 1;
}

int XSetWindowBorderPixmap(Display* display, Window window, Pixmap border_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderPixmap.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    // TODO: has no effect
    return 1;
}

int XSetWindowColormap(Display* display, Window window, Colormap colormap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowColormap.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    if (window != SCREEN_WINDOW) {
        GET_WINDOW_STRUCT(window)->colormap = colormap;
    }
    return 1;
}

int XChangeWindowAttributes(Display* display, Window window, unsigned long valueMask,
                             XSetWindowAttributes *attributes) {
    // https://tronche.com/gui/x/xlib/window/XChangeWindowAttributes.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (IS_TOP_LEVEL(window) && HAS_VALUE(valueMask, CWCursor)) {
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
    return 1;
}

Window XRootWindow(Display* display, int screen_number) {
    // https://tronche.com/gui/x/xlib/display/display-macros.html#RootWindow
//    SET_X_SERVER_REQUEST(display, XCB_);
    (void) display;
    (void) screen_number;
    return SCREEN_WINDOW;
}

int XMoveResizeWindow(Display* display, Window window, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XMoveResizeWindow.html
    if (XMoveWindow(display, window, x, y) && XResizeWindow(display, window, width, height)) {
        return 1;
    }
    return 0;
}

int XSetWindowBorderWidth(Display* display, Window window, unsigned int width) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderWidth.html
    SET_X_SERVER_REQUEST(display, X_ChangeWindowAttributes);
    // TODO: has no effect
    return 1;
}

Status XQueryTree(Display* display, Window window, Window* root_return, Window* parent_return,
                  Window** children_return, unsigned int* nchildren_return) {
    // https://tronche.com/gui/x/xlib/window-information/XQueryTree.html
    SET_X_SERVER_REQUEST(display, X_QueryTree);
    TYPE_CHECK(window, WINDOW, display, 0);
    *root_return = SCREEN_WINDOW;
    *parent_return = GET_PARENT(window);
    *nchildren_return = GET_WINDOW_STRUCT(window)->children.length;
    *children_return = malloc(sizeof(Window) * (*nchildren_return));
    if (*children_return == NULL) return 0;
    memcpy(children_return, GET_CHILDREN(window), sizeof(Window) * (*nchildren_return));
    return 1;
}
