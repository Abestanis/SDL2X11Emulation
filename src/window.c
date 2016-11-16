#include <X11/Xlib.h>
#include "window.h"
#include "errors.h"
#include "drawing.h"
#include "atoms.h"
#include "events.h"
#include "display.h"

// TODO: Cover cases where top-level window is re-parented and window is converted to top-level window
// TODO: Make child list ordered and with no NULL objects

void XDestroyWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XDestroyWindow.html
    SET_X_SERVER_REQUEST(display, XCB_DESTROY_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    if (window == SCREEN_WINDOW) return;
    destroyWindow(display, window, True);
}

Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width,
                     unsigned int height, unsigned int border_width, int depth, unsigned int class,
                     Visual* visual, unsigned long valueMask, XSetWindowAttributes* attributes) {
    // https://tronche.com/gui/x/xlib/window/XCreateWindow.html
    SET_X_SERVER_REQUEST(display, XCB_CREATE_WINDOW);
    TYPE_CHECK(parent, WINDOW, display, NULL);
    Bool inputOnly = (class == InputOnly || (class == CopyFromParent && IS_INPUT_ONLY(parent)));
    if (inputOnly && border_width != 0) {
        fprintf(stderr, "Bad argument: Given class is InputOnly but border_with is not 0 in XCreateWindow!\n");
        handleError(0, display, NULL, 0, BadMatch, 0);
        return NULL;
    }
    Window windowID = malloc(sizeof(Window));
    if (windowID == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window id in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    WindowStruct* windowStruct = malloc(sizeof(WindowStruct));
    if (windowStruct == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window struct in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        free(windowID);
        return NULL;
    }
    windowID->type = WINDOW;
    windowID->dataPointer = windowStruct;
    initWindowStruct(windowStruct, x, y, width, height, visual, NULL, inputOnly, 0, NULL);
    windowStruct->depth = depth;
    windowStruct->borderWidth = border_width;
    if (!addChildToWindow(parent, windowID)) {
        fprintf(stderr, "Out of memory: Could not increase size of parent's child list in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        free(windowStruct);
        free(windowID);
        return NULL;
    }
    int visualClass;
#if defined(__cplusplus) || defined(c_plusplus)
    visualClass = visual->c_class;
#else
    visualClass = visual->class;
#endif
    windowStruct->colormap = (Colormap) XCreateColormap(display, windowID, visual,
                                                         visualClass == StaticGray ||
                                                         visualClass == StaticColor ||
                                                         visualClass == TrueColor ?
                                                         AllocAll : AllocNone);
    display->request = XCB_CREATE_WINDOW;
    if (windowStruct->colormap == NULL) {
        fprintf(stderr, "Out of memory: Could not allocate the window colormap in XCreateWindow!\n");
        handleOutOfMemory(0, display, 0, 0);
        free(windowStruct);
        free(windowID);
        return NULL;
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

void XConfigureWindow(Display* display, Window window, unsigned int value_mask,
                      XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/window/XConfigureWindow.html
    SET_X_SERVER_REQUEST(display, XCB_CONFIGURE_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    configureWindow(display, window, value_mask, values);
}

Status XReconfigureWMWindow(Display* display, Window window, int screen_number,
                            unsigned int value_mask, XWindowChanges* values) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XReconfigureWMWindow.html
    XConfigureWindow(display, window, value_mask, values);
    return 1;
}

Status XIconifyWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XIconifyWindow.html
    SET_X_SERVER_REQUEST(display, XCB_ICONIFY_WINDOW);
    TYPE_CHECK(window, WINDOW, display, 0);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_MinimizeWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
    return 1;
}

Status XSetWMColormapWindows(Display* display, Window window, Window* colormap_windows, int count) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetWMColormapWindows.html
    SET_X_SERVER_REQUEST(display, XCB_SET_WM_COLOR_MAPS_WINDOWS);
    // TODO: Should we do sth. with the information?
    TYPE_CHECK(window, WINDOW, display, 0);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    windowStruct->colormapWindowsCount = count;
    windowStruct->colormapWindows = colormap_windows;
    return 1;
}

Status XGetWMColormapWindows(Display* display, Window window, Window** colormap_windows_return, int* count_return) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XGetWMColormapWindows.html
    SET_X_SERVER_REQUEST(display, XCB_GET_WM_COLOR_MAPS_WINDOWS);
    TYPE_CHECK(window, WINDOW, display, 0);
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
//    SET_X_SERVER_REQUEST(display, XCB_);
    // TODO: Ignored for now
}

void XMapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XMapWindow.html
    SET_X_SERVER_REQUEST(display, XCB_MAP_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    if (GET_WINDOW_STRUCT(window)->mapState == Mapped || GET_WINDOW_STRUCT(window)->mapState == MapRequested) { return; }
    if (!GET_WINDOW_STRUCT(window)->overrideRedirect && HAS_EVENT_MASK(GET_PARENT(window), SubstructureRedirectMask)) {
        postEvent(display, window, MapRequest);
        return;
    }
    if (IS_TOP_LEVEL(window)) {
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) { return; }
        fprintf(stderr, "Mapping Window %p\n", window);
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
            handleError(0, display, NULL, 0, BadMatch, 0);
            return;
        }
        registerWindowMapping(window, SDL_GetWindowID(sdlWindow));
        GPU_Target* renderTarget = GPU_CreateTargetFromWindow(SDL_GetWindowID(sdlWindow));
        if (renderTarget == NULL) {
            fprintf(stderr, "GPU_CreateTargetFromWindow failed in XMapWindow: %s\n", GPU_PopErrorCode().details);
            handleError(0, display, NULL, 0, BadMatch, 0);
            return;
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
                return;
            }
            GET_WINDOW_STRUCT(window)->mapState = Mapped;
        } else { /* Parent not mapped */
            // mapRequestedChildren will do all the work
            // TODO: Have a look at this: https://tronche.com/gui/x/xlib/window/map.html
            GET_WINDOW_STRUCT(window)->mapState = MapRequested;
            return;
        }
    }
    postEvent(display, window, MapNotify);
    mapRequestedChildren(display, window);
    #ifdef DEBUG_WINDOWS
    printWindowsHierarchy();
    #endif
}

void XUnmapWindow(Display* display, Window window) {
    // https://tronche.com/gui/x/xlib/window/XUnmapWindow.html
    SET_X_SERVER_REQUEST(display, XCB_UNMAP_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (windowStruct->renderTarget != NULL) {
        GPU_FreeTarget(windowStruct->renderTarget);
        windowStruct->renderTarget = NULL;
    }
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_DestroyWindow(windowStruct->sdlWindow);
        windowStruct->sdlWindow = NULL;
    }
    // TODO: Expose events
    // TODO: Change subwindow state to MapRequested?
    windowStruct->mapState = UnMapped;
    postEvent(display, window, UnmapNotify, False);
}

Status XWithdrawWindow(Display* display, Window window, int screen_number) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XWithdrawWindow.html
    SET_X_SERVER_REQUEST(display, XCB_UNMAP_WINDOW);
    TYPE_CHECK(window, WINDOW, display, 1);
    XUnmapWindow(display, window);
    return 1;
}

void XStoreName(Display* display, Window window, char* window_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XStoreName.html
    SET_X_SERVER_REQUEST(display, XCB_STORE_NAME);
    TYPE_CHECK(window, WINDOW, display);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_SetWindowTitle(GET_WINDOW_STRUCT(window)->sdlWindow, window_name);
    } else {
        char* windowName = malloc(sizeof(char) * (strlen(window_name) + 1));
        if (windowName == NULL) {
            handleError(0, display, window, 0, BadAlloc, 0);
            return;
        }
        strcpy(windowName, window_name);
        GET_WINDOW_STRUCT(window)->windowName = windowName;
    }
}

void XSetIconName(Display* display, Window window, char* icon_name) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetIconName.html
//    SET_X_SERVER_REQUEST(display, XCB_);
    // There is not really anything to do here // TODO: Check if this is true
}

void XMoveWindow(Display* display, Window window, int x, int y) {
    // https://tronche.com/gui/x/xlib/window/XMoveWindow.html
    SET_X_SERVER_REQUEST(display, XCB_MOVE_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    if (window != SCREEN_WINDOW) {
        WindowStruct *windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowPosition(windowStruct->sdlWindow, x, y);
            SDL_GetWindowPosition(windowStruct->sdlWindow, &windowStruct->x, &windowStruct->y);
        } else {
            windowStruct->x = x;
            windowStruct->y = y;
        }
    }
}

void XResizeWindow(Display* display, Window window, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XResizeWindow.html
    SET_X_SERVER_REQUEST(display, XCB_RESIZE_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    XWindowChanges changes;
    changes.width = width;
    changes.height = height;
    configureWindow(display, window, CWWidth | CWHeight, &changes);
}

void XReparentWindow(Display* display, Window window, Window parent, int x, int y) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XReparentWindow.html
    SET_X_SERVER_REQUEST(display, XCB_REPARENT_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    TYPE_CHECK(parent, WINDOW, display);
    if (window == parent) {
        fprintf(stderr, "Invalid parameter: Can not add window to itself in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return;
    } else if (IS_INPUT_ONLY(parent) && !IS_INPUT_ONLY(window)) {
        fprintf(stderr, "Invalid parameter: Can not add InputOutput window to InputOnly window in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return;
    } else if (isParent(window, parent)) {
        fprintf(stderr, "Invalid parameter: Can not add window to one of it's childs in XReparentWindow!\n");
        handleError(0, display, window, 0, BadMatch, 0);
        return;
    }
    MapState mapState = GET_WINDOW_STRUCT(window)->mapState;
    XUnmapWindow(display, window);
    Window oldParent = GET_PARENT(window);
    removeChildFromParent(window);
    if (!addChildToWindow(parent, window)) {
        fprintf(stderr, "Out of memory: Failed to reattach window in XReparentWindow!\n");
        return;
    }
    XMoveWindow(display, window, x, y); // TODO: Do this without generating events
    postEvent(display, window, ReparentNotify, oldParent);
    if (mapState != UnMapped) {
        XMapWindow(display, window);
    }
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
    SET_X_SERVER_REQUEST(display, XCB_TRANSLATE_COORDINATES);
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
        destParents[0] = NULL;
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
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_PROPERTY);
    TYPE_CHECK(window, WINDOW, display);
    fprintf(stderr, "Changing window property %lu (%s).\n", property, (char*) XGetAtomName(display, property));
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, 0);
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
            handleError(0, display, NULL, 0, BadMatch, 0);
            return;
        }
    } else {
        windowProperty = malloc(sizeof(WindowProperty));
        if (windowProperty == NULL) {
            fprintf(stderr, "Out of memory: Failed to allocate space for new "
                    "windowProperty in XChangeProperty!\n");
            handleOutOfMemory(0, display, 0, 0);
            return;
        }
        if (windowStruct->propertySize < windowStruct->propertyCount + 1) {
            unsigned int newSize;
            WindowProperty* newProperties = increasePropertySize(windowStruct->properties,
                                                                 windowStruct->propertySize, &newSize);
            if (newProperties == NULL) {
                fprintf(stderr, "Out of memory: Failed to increase property list size in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
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
                fprintf(stderr, "Out of memory: Failed to allocate space for combined data "
                                "in XChangeProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
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
            handleError(0, display, NULL, 0, BadMatch, 0);
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
        if (windowStruct->icon != NULL) {
            SDL_FreeSurface(windowStruct->icon);
        }
        windowStruct->icon = icon;
        if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
            SDL_SetWindowIcon(windowStruct->sdlWindow, icon);
        }
    }
}

void XDeleteProperty(Display* display, Window window, Atom property) {
    // https://tronche.com/gui/x/xlib/window-information/XDeleteProperty.html
    SET_X_SERVER_REQUEST(display, XCB_DELETE_PROPERTY);
    TYPE_CHECK(window, WINDOW, display);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, 0);
        return;
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
    SET_X_SERVER_REQUEST(display, XCB_GET_PROPERTY);
    TYPE_CHECK(window, WINDOW, display, BadWindow);
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    if (!isValidAtom(property)) {
        handleError(0, display, NULL, 0, BadAtom, 0);
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
                handleError(0, display, NULL, 0, BadValue , 0);
                return BadValue;
            }
            int length = long_offset * 4 + long_length * 4 < windowProperty->dataLength ?
                         long_length * 4 : long_offset * 4 - windowProperty->dataLength;
            *prop_return = malloc(sizeof(unsigned char) * (length + 1));
            if (*prop_return == NULL) {
                fprintf(stderr, "Out of memory: Failed to allocate space for the retrun value "\
                        "in XGetWindowProperty!\n");
                handleOutOfMemory(0, display, 0, 0);
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
    SET_X_SERVER_REQUEST(display, XCB_RAISE_WINDOW);
    TYPE_CHECK(window, WINDOW, display);
    if (IS_MAPPED_TOP_LEVEL_WINDOW(window)) {
        SDL_RaiseWindow(GET_WINDOW_STRUCT(window)->sdlWindow);
    }
    // TODO: Rearrange child in child list of parent.
}

Status XGetWindowAttributes(Display* display, Window window,
                            XWindowAttributes* window_attributes_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetWindowAttributes.html
    SET_X_SERVER_REQUEST(display, XCB_GET_WINDOW_ATTRIBUTES);
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
        if (windowStruct->parent == NULL) {
            window_attributes_return->map_state = IsUnmapped;
        } else {
            window_attributes_return->map_state = IsViewable;
        }
    }
    window_attributes_return->depth = SDL_SURFACE_DEPTH;
    window_attributes_return->colormap = GET_WINDOW_STRUCT(window)->colormap;
    return 1;
}

void XSetWindowBackground(Display* display, Window window, unsigned long background_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackground.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    if (window != SCREEN_WINDOW) {
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background of an InputOnly "
                    "window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, 0);
            return;
        }
        GET_WINDOW_STRUCT(window)->backgroundColor = background_pixel;
    }
}

void XSetWindowBackgroundPixmap(Display* display, Window window, Pixmap background_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBackgroundPixmap.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    if (window != SCREEN_WINDOW) {
        WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
        if (IS_INPUT_ONLY(window)) {
            fprintf(stderr, "Invalid parameter: Can not change the background Pixmap of an "
                    "InputOnly window in XChangeWindowAttributes!\n");
            handleError(0, display, window, 0, BadMatch, 0);
            return;
        }
        Pixmap previous = windowStruct->background;
        if (background_pixmap == (Pixmap) ParentRelative) {
            windowStruct->background = GET_PARENT(window) == NULL ? NULL :
                                             GET_WINDOW_STRUCT(GET_PARENT(window))->background;
        } else {
            TYPE_CHECK(background_pixmap, PIXMAP, display);
            windowStruct->background = background_pixmap;
        }
        if (previous != NULL && previous != None) {
            XFreePixmap(display, previous);
        }
    }
}

void XSetWindowBorder(Display* display, Window window, unsigned long border_pixel) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorder.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    //TODO: has no effect
}

void XSetWindowBorderPixmap(Display* display, Window window, Pixmap border_pixmap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderPixmap.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    // TODO: has no effect
}

void XSetWindowColormap(Display* display, Window window, Colormap colormap) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowColormap.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    if (window != SCREEN_WINDOW) {
        GET_WINDOW_STRUCT(window)->colormap = colormap;
    }
}

void XChangeWindowAttributes(Display* display, Window window, unsigned long valueMask,
                             XSetWindowAttributes *attributes) {
    // https://tronche.com/gui/x/xlib/window/XChangeWindowAttributes.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    TYPE_CHECK(window, WINDOW, display);
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
}

Window XRootWindow(Display* display, int screen_number) {
    // https://tronche.com/gui/x/xlib/display/display-macros.html#RootWindow
//    SET_X_SERVER_REQUEST(display, XCB_);
    (void) display;
    (void) screen_number;
    return SCREEN_WINDOW;
}

void XMoveResizeWindow(Display* display, Window window, int x, int y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/window/XMoveResizeWindow.html
    XMoveWindow(display, window, x, y);
    XResizeWindow(display, window, width, height);
}

void XSetWindowBorderWidth(Display* display, Window window, unsigned int width) {
    // https://tronche.com/gui/x/xlib/window/XSetWindowBorderWidth.html
    SET_X_SERVER_REQUEST(display, XCB_CHANGE_WINDOW_ATTRIBUTES);
    // TODO: has no effect
}

Status XQueryTree(Display* display, Window window, Window* root_return, Window* parent_return,
                  Window** children_return, unsigned int* nchildren_return) {
    // https://tronche.com/gui/x/xlib/window-information/XQueryTree.html
    SET_X_SERVER_REQUEST(display, XCB_QUERY_TREE);
    TYPE_CHECK(window, WINDOW, display, 0);
    *root_return = SCREEN_WINDOW;
    *parent_return = GET_PARENT(window);
    *nchildren_return = 0;
    Window* children = GET_CHILDREN(window);
    int i, counter = 0;
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            (*nchildren_return)++;
        }
    }
    *children_return = malloc(sizeof(Window) * (*nchildren_return));
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            *children_return[counter++] = children[i];
        }
    }
    return 1;
}
