//
// Created by Sebastian on 08.10.2016.
//

#include "windowDebug.h"

#ifdef DEBUG_WINDOWS

#include <unistd.h>
#include "window.h"
#include "drawing.h"
#include "util.h"


void printWindowHierarchyOfChild(Window window, char* prepend, int prependLen) {
    size_t i;
    Window* children = GET_CHILDREN(window);
    char* childPrepend = malloc(sizeof(char) * (prependLen + 2));
    int numChildren = GET_WINDOW_STRUCT(window)->children.length;
    strcpy(childPrepend, prepend);
    char* charPointer = childPrepend + prependLen;
    *(charPointer + 1) = '\0';
    for (i = 0; i < numChildren; i++) {
        int x, y, w, h;
        GET_WINDOW_POS(children[i], x, y);
        GET_WINDOW_DIMS(children[i], w, h);
        char* mapState;
        switch (GET_WINDOW_STRUCT(children[i])->mapState) {
            case UnMapped: mapState = "UnMapped"; break;
            case Mapped: mapState = "Mapped"; break;
            case MapRequested: mapState = "MapRequested"; break;
            default: mapState = "Unknown";
        }
        printf("%s+- Window (address: %lu, id: 0x%08lx, x: %d, y: %d, %dx%d, state: %s)",
               prepend, children[i], GET_WINDOW_STRUCT(children[i])->debugId, x, y, w, h, mapState);
               
        if (GET_WINDOW_STRUCT(children[i])->renderTarget != NULL) {
            printf(", rendererTarget = %p", GET_WINDOW_STRUCT(children[i])->renderTarget);
        }
        if (GET_WINDOW_STRUCT(children[i])->sdlWindow != NULL) {
            printf(", sdlWindow = %d", SDL_GetWindowID(GET_WINDOW_STRUCT(children[i])->sdlWindow));
        }
        if (GET_WINDOW_STRUCT(children[i])->unmappedContent != NULL) {
            GPU_Image* unmappedContent = GET_WINDOW_STRUCT(children[i])->unmappedContent;
            printf(", unmappedContent = %p (%d x %d)", unmappedContent, unmappedContent->w, unmappedContent->h);
        }
        printf("\n");
        *charPointer = (char) (i != numChildren - 1 ? ' ' : '|');
        printWindowHierarchyOfChild(children[i], childPrepend, prependLen + 1);
    }
    free(childPrepend);
}

void printWindowsHierarchy() {
    printf("- SCREEN_WINDOW (address: %lu, id = 0x%08lx)\n", SCREEN_WINDOW, GET_WINDOW_STRUCT(SCREEN_WINDOW)->debugId);
    printWindowHierarchyOfChild(SCREEN_WINDOW, "", 0);
    fflush(stdout);
}

void drawChildDebugBorder(Window window) {
    size_t i;
    unsigned int w, h;
    unsigned long windowColor;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    LOG("getWindowRenderTarget of window %lu in %s.\n", window, __func__);
    GPU_Target* renderTarget = getWindowRenderTarget(window);
    if (renderTarget != NULL) {
        GET_WINDOW_DIMS(window, w, h);
        windowColor = windowStruct->debugId;
        SDL_Color color = {
                GET_RED_FROM_COLOR(windowColor),
                GET_GREEN_FROM_COLOR(windowColor),
                GET_BLUE_FROM_COLOR(windowColor),
                (Uint8) (windowStruct->mapState == Mapped ? 0xFF : 0x0F)
        };
        GPU_Rectangle(renderTarget, 0, 0, w, h, color);
    }
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < GET_WINDOW_STRUCT(window)->children.length; i++) {
        drawChildDebugBorder(children[i]);
    }
}

void drawWindowsDebugBorder() {
    size_t i;
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->children.length; i++) {
        if (GET_WINDOW_STRUCT(children[i])->mapState == Mapped) {
            drawChildDebugBorder(children[i]);
            LOG("getWindowRenderTarget of window %lu in %s.\n", children[i], __func__);
            GPU_Flip(getWindowRenderTarget(children[i]));
        }
    }
}

void drawChildDebugSurfacePlane(Window window) {
    size_t i;
    long windowColor;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    LOG("getWindowRenderTarget of window %lu in %s.\n", window, __func__);
    GPU_Target* renderTarget = getWindowRenderTarget(window);
    if (renderTarget != NULL) {
        windowColor = windowStruct->debugId;
        SDL_Color color = {
                GET_RED_FROM_COLOR(windowColor),
                GET_GREEN_FROM_COLOR(windowColor),
                GET_BLUE_FROM_COLOR(windowColor),
                0x03
        };
        int x, y, w, h;
        GET_WINDOW_POS(window, x, y);
        GET_WINDOW_DIMS(window, w, h);
        GPU_RectangleFilled(renderTarget, x, y, w, h, color);
    } else {
        LOG("Failed to get renderer target for window %lu in %s\n", window, __func__);
    }
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < windowStruct->children.length; i++) {
        if (GET_WINDOW_STRUCT(children[i])->mapState == Mapped) {
            drawChildDebugSurfacePlane(children[i]);
        }
    }
}

void drawWindowsDebugSurfacePlane() {
    size_t i;
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->children.length; i++) {
        if (GET_WINDOW_STRUCT(children[i])->renderTarget != NULL) {
            drawChildDebugSurfacePlane(children[i]);
            GPU_Flip(GET_WINDOW_STRUCT(children[i])->renderTarget);
        }
    }
}

#endif /* DEBUG_WINDOWS */
