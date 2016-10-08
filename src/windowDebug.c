//
// Created by Sebastian on 08.10.2016.
//

#include "windowDebug.h"
#ifdef DEBUG_WINDOWS


#include <unistd.h>
#include <SDL.h>

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

void drawWindowDebugViewForChild(SDL_Renderer* renderer, Window window, int absParentX, int absParentY) {
    int i;
    SDL_Rect windowRect;
    long drawColor = GET_WINDOW_STRUCT(window)->debugId;
    GET_WINDOW_POS(window, windowRect.x, windowRect.y);
    GET_WINDOW_DIMS(window, windowRect.w, windowRect.h);
    windowRect.x += absParentX;
    windowRect.y += absParentY;
    SDL_SetRenderDrawColor(renderer, ((drawColor >> 24) & 0xFF) * 0.9, ((drawColor >> 16) & 0xFF) * 0.9, ((drawColor >> 8) & 0xFF) * 0.9, 0xFF);
    SDL_RenderDrawRect(renderer, &windowRect);
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] != NULL) {
            drawWindowDebugViewForChild(renderer, children[i], windowRect.x, windowRect.y);
        }
    }
}

void drawWindowDebugView() {
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    int i, j;
    long windowColor;
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->sdlRenderer != NULL) {
            windowColor = GET_WINDOW_STRUCT(children[i])->debugId;
            WindowStruct* windowStruct = GET_WINDOW_STRUCT(children[i]);
            SDL_RenderSetViewport(windowStruct->sdlRenderer, NULL);
            SDL_Rect windowRect;
            GET_WINDOW_POS(children[i], windowRect.x, windowRect.y);
            GET_WINDOW_DIMS(children[i], windowRect.w, windowRect.h);
            SDL_SetRenderDrawColor(windowStruct->sdlRenderer, (windowColor >> 24) & 0xFF,
                                   (windowColor >> 16) & 0xFF, (windowColor >> 8) & 0xFF, 0xFF);
            SDL_RenderDrawRect(windowStruct->sdlRenderer, &windowRect);
            Window* topLevelWindowChildren = GET_CHILDREN(children[i]);
            for (j = 0; j < windowStruct->childSpace; j++) {
                if (topLevelWindowChildren[j] != NULL) {
                    drawWindowDebugViewForChild(windowStruct->sdlRenderer, topLevelWindowChildren[j], 0, 0);
                }
            }
            SDL_RenderPresent(windowStruct->sdlRenderer);
        }
    }
}

void drawDebugWindowChildSurfacePlanes(Window child) {
    int i;
    long windowColor = GET_WINDOW_STRUCT(child)->debugId;
    SDL_Renderer* renderer = getWindowRenderer(child);
    SDL_Rect windowRect;
    SDL_RenderGetViewport(renderer, &windowRect);
    windowRect.x = 0;
    windowRect.y = 0;
    SDL_SetRenderDrawColor(renderer, (windowColor >> 24) & 0xFF, (windowColor >> 16) & 0xFF,
                           (windowColor >> 8) & 0xFF, 0x55);
    SDL_RenderFillRect(renderer, &windowRect);
    Window* children = GET_CHILDREN(child);
    for (i = 0; i < GET_WINDOW_STRUCT(child)->childSpace; i++) {
        if (children[i] != NULL) {
            drawDebugWindowChildSurfacePlanes(children[i]);
        }
    }
}

void drawDebugWindowSurfacePlanes() {
    Window* children = GET_CHILDREN(SCREEN_WINDOW);
    int i;
    for (i = 0; i < GET_WINDOW_STRUCT(SCREEN_WINDOW)->childSpace; i++) {
        if (children[i] != NULL && GET_WINDOW_STRUCT(children[i])->sdlRenderer != NULL) {
            SDL_Renderer* renderer = GET_WINDOW_STRUCT(children[i])->sdlRenderer;
            SDL_BlendMode oldBlendMode;
            SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            drawDebugWindowChildSurfacePlanes(children[i]);
            SDL_RenderPresent(renderer);
            SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);
        }
    }
}

#endif /* DEBUG_WINDOWS */
