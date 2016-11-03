//
// Created by Sebastian on 08.10.2016.
//

#include "windowDebug.h"
#ifdef DEBUG_WINDOWS


#include <unistd.h>
#include "drawing.h"

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
            *charPointer = childCounter == 1 ? ' ' : '|';
            fflush(stdout);
            printWindowHierarchyOfChild(children[i], childPrepend, prependLen + 1);
            childCounter--;
        }
    }
    free(childPrepend);
}

void printWindowsHierarchy() {
    printf("- SCREEN_WINDOW (adress: %p, id = 0x%08lx)\n", SCREEN_WINDOW, GET_WINDOW_STRUCT(SCREEN_WINDOW)->debugId);
    printWindowHierarchyOfChild(SCREEN_WINDOW, "", 0);
    fflush(stdout);
}

void drawChildDebugBorder(Window window) {
    size_t i;
    unsigned int w, h;
    unsigned long windowColor;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    fprintf(stderr, "getWindowRenderTarget of window %p in %s.\n", window, __func__);
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
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            drawChildDebugBorder(children[i]);
        }
    }
}

void drawWindowsDebugBorder() {
    size_t i;
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->mapState == Mapped) {
            drawChildDebugBorder(children[i]);
            fprintf(stderr, "getWindowRenderTarget of window %p in %s.\n", children[i], __func__);
            GPU_Flip(getWindowRenderTarget(children[i]));
        }
    }
}

void drawChildDebugSurfacePlane(Window window) {
    size_t i;
    long windowColor;
    WindowStruct* windowStruct = GET_WINDOW_STRUCT(window);
    fprintf(stderr, "getWindowRenderTarget of window %p in %s.\n", window, __func__);
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
        fprintf(stderr, "Failed to get renderer target for window %p in %s\n", window, __func__);
    }
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < windowStruct->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->mapState == Mapped) {
            drawChildDebugSurfacePlane(children[i]);
        }
    }
}

void drawWindowsDebugSurfacePlane() {
    size_t i;
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->renderTarget != NULL) {
            drawChildDebugSurfacePlane(children[i]);
            GPU_Flip(GET_WINDOW_STRUCT(children[i])->renderTarget);
        }
    }
}

#endif /* DEBUG_WINDOWS */
