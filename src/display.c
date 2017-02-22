#include <math.h>
#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
#include "X11/Xutil.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "window.h"
#include "errors.h"
#include "events.h"
#include "colors.h"
#include "drawing.h"
#include "display.h"
#include "atoms.h"
#include <jni.h>
#include <SDL_gpu.h>
#include <X11/X.h>
#include <X11/Xutil.h>

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    setenv("DISPLAY", ":0", 0);
    return JNI_VERSION_1_4;
}

void __attribute__((constructor)) _init() {
    setenv("DISPLAY", ":0", 0);
}

int numDisplaysOpen = 0;
// TODO: or SDL_GetCurrentVideoDriver
static char* vendor = "Vendor";
static const int releaseVersion = 1;
Visual* trueColorVisual = NULL;

int XCloseDisplay(Display* display) {
    // https://tronche.com/gui/x/xlib/display/XCloseDisplay.html
    if (numDisplaysOpen == 1) {
        freeAtomStorage();
        destroyScreenWindow(display);
        TTF_Quit();
        GPU_Quit();
        SDL_Quit();
    }
    if (numDisplaysOpen > 0) {
        numDisplaysOpen--;
    }
    if (GET_DISPLAY(display)->nscreens > 0) {
        int i;
        for (i = 0; i < GET_DISPLAY(display)->nscreens; ++i) {
            free(&GET_DISPLAY(display)->screens[i]);
        }
    }
    free(display);
    return 0;
}

Display* XOpenDisplay(_Xconst char* display_name) {
    // https://tronche.com/gui/x/xlib/display/opening.html
    if (trueColorVisual == NULL) {
        trueColorVisual = malloc(sizeof(Visual));
        if (trueColorVisual == NULL) {
            fprintf(stderr, "Out of memory: Failed to allocate memory for the trueColorVisual in XOpenDisplay!\n");
            return NULL;
        }
        trueColorVisual->visualid = 0;	/* visual id of this visual | TODO: Is this important? */
        #if defined(__cplusplus) || defined(c_plusplus)
            trueColorVisual->c_class = TrueColor;
        #else
            trueColorVisual->class = TrueColor;
        #endif
        trueColorVisual->red_mask = 0xFF000000;
        trueColorVisual->green_mask = 0x00FF0000;
        trueColorVisual->blue_mask = 0x0000FF00;
        trueColorVisual->bits_per_rgb = sizeof(SDL_Color);
        // TODO: This might be wrong
        trueColorVisual->map_entries = 16581375 /* (255 * 255 * 255) */ ;	/* color map entries */
    }
    
    Display* display = malloc(sizeof(Display));
    if (display == NULL) {
        fprintf(stderr, "Out of memory: Failed to allocate memory for Display struct in XOpenDisplay!");
        return NULL;
    }
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
            fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
            free(display);
            return NULL;
        }
//        SDL_SetHintWithPriority("SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH", "0", SDL_HINT_OVERRIDE);
    }
    if (!TTF_WasInit()) {
        if (TTF_Init() == -1) {
            fprintf(stderr, "Failed to initialize SDL_TTF: %s\n", TTF_GetError());
            free(display);
            return NULL;
        }
    }
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    numDisplaysOpen++;
    
    display->qlen = 0;
    int eventFd = initEventPipe(display);
    if (eventFd < 0) {
        display->nscreens = 0;
        XCloseDisplay(display);
        return NULL;
    }
    display->fd = eventFd;
    display->proto_major_version = X_PROTOCOL;
    display->proto_minor_version = X_PROTOCOL_REVISION;
    display->vendor = vendor;
    display->release = releaseVersion;
    display->request = X_NoOperation;
    display->display_name = (char*) display_name;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    display->byte_order = MSBFirst;
#else
    display->byte_order = LSBFirst;
#endif
    display->default_screen = 0; // TODO: Investigate here, see SDL_GetCurrentVideoDisplay();
    display->nscreens = SDL_GetNumVideoDisplays();
    if (display->nscreens < 0) {
        fprintf(stderr, "Failed to get the number of screens: %s\n", SDL_GetError());
        XCloseDisplay(display);
        return NULL;
    }
    display->screens = malloc(sizeof(Screen) * display->nscreens);
    if (display->screens == NULL) {
        fprintf(stderr, "Failed to get the number of screens: %s\n", SDL_GetError());
        XCloseDisplay(display);
        return NULL;
    }
    int screenIndex;
    for (screenIndex = 0; screenIndex < display->nscreens; screenIndex++) {
        Screen* screen = &display->screens[screenIndex];
        SDL_DisplayMode displayMode;
        if (SDL_GetDesktopDisplayMode(screenIndex, &displayMode) != 0) {
            int nAllocatedData = display->nscreens;
            int i;
            for (i = screenIndex; i < nAllocatedData; i++) {
                free(&display->screens[nAllocatedData]);
            }
            display->nscreens = screenIndex;
            XCloseDisplay(display);
            fprintf(stderr, "Failed to get the display mode in XOpenDisplay: %s\n", SDL_GetError());
            return NULL;
        }
        screen->root_visual = trueColorVisual;
        screen->display = display;
        screen->width   = displayMode.w;
        screen->height  = displayMode.h;
        #if SDL_VERSION_ATLEAST(2, 0, 4)
        // Calculate the values in millimeters
        float h_dpi, v_dpi;
        if (SDL_GetDisplayDPI(screenIndex, NULL, &h_dpi, &v_dpi) != 0) {
            fprintf(stderr, "Warning: SDL_GetDisplayDPI failed, using pixel values for mm values: %s",
                    SDL_GetError());
            screen->mwidth  = displayMode.w;
            screen->mheight = displayMode.h;
        } else {
            screen->mwidth  = (int) roundf((displayMode.w * v_dpi) / 25.4f);
            screen->mheight = (int) roundf((displayMode.h * h_dpi) / 25.4f);
        }
        #else
        screen->mwidth  = displayMode.w;
        screen->mheight = displayMode.h;
        #endif
        screen->root = SCREEN_WINDOW;
        // TODO: Need real values here
        screen->root_depth = sizeof(SDL_Color); /* bits per pixel */
        screen->white_pixel = 0xFFFFFFFF;
        screen->black_pixel = 0x000000FF;
        screen->cmap = REAL_COLOR_COLORMAP;
    }
    if (SCREEN_WINDOW == None) {
        if (initScreenWindow(display) != True) {
            fprintf(stderr, "XOpenDisplay: Initializing the screen window failed!\n");
            XCloseDisplay(display);
            return NULL;
        }
        for (screenIndex = 0; screenIndex < display->nscreens; screenIndex++) {
            display->screens[screenIndex].root = SCREEN_WINDOW;
        }
    }
    GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlWindow = SDL_CreateWindow(NULL, 0, 0, 10, 10, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    if (GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlWindow == NULL) {
        fprintf(stderr, "XOpenDisplay: Initializing the SDL screen window failed: %s!\n", SDL_GetError());
        XCloseDisplay(display);
        return NULL;
    }
    GPU_SetInitWindow(SDL_GetWindowID(GET_WINDOW_STRUCT(SCREEN_WINDOW)->sdlWindow));
    GET_WINDOW_STRUCT(SCREEN_WINDOW)->renderTarget = GPU_Init(0, 0, 0);
    if (GET_WINDOW_STRUCT(SCREEN_WINDOW)->renderTarget == NULL) {
        fprintf(stderr, "XOpenDisplay: Initializing SDL_gpu failed!\n");
        XCloseDisplay(display);
        return NULL;
    }
    return display;
}

int XBell(Display* display, int percent) {
    // https://tronche.com/gui/x/xlib/input/XBell.html
    SET_X_SERVER_REQUEST(display, X_Bell);
    if (-100 > percent || 100 < percent) {
        handleError(0, display, None, 0, BadValue, 0);
    } else {
        // TODO: Should it be implemented with audio or haptic feedback?
        printf("BLING!\n");
    }
    return 1;
}

int XSync(Display *display, Bool discard) {
    // https://tronche.com/gui/x/xlib/event-handling/XSync.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    flipScreen();
    return 1;
}

int XConvertSelection(Display* display, Atom selection, Atom target, Atom property,
                       Window requestor, Time time) {
    // https://tronche.com/gui/x/xlib/window-information/XConvertSelection.html
    // http://www.man-online.org/page/3-XConvertSelection/
    SET_X_SERVER_REQUEST(display, X_ConvertSelection);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XSetSelectionOwner(Display *display, Atom selection, Window owner, Time time) {
    // https://tronche.com/gui/x/xlib/window-information/XSetSelectionOwner.html
    SET_X_SERVER_REQUEST(display, X_SetSelectionOwner);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int (*XSynchronize(Display *display, Bool onoff))(Display* dsp) {
    // https://tronche.com/gui/x/xlib/event-handling/protocol-errors/XSynchronize.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return NULL;
}

int XNoOp(Display *display) {
    // https://tronche.com/gui/x/xlib/display/XNoOp.html
    SET_X_SERVER_REQUEST(display, X_NoOperation);
//    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XGrabServer(Display *display) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XGrabServer.html
    SET_X_SERVER_REQUEST(display, X_GrabServer);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XUngrabServer(Display *display) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XUngrabServer.html
    SET_X_SERVER_REQUEST(display, X_UngrabServer);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
} 

XHostAddress* XListHosts(Display *display, int *nhosts_return, Bool *state_return) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/controlling-host-access/XListHosts.html
    SET_X_SERVER_REQUEST(display, X_ListHosts);
    const static char* LOCAL_HOST = "127.0.0.1";
    *state_return = True;
    XHostAddress* host = malloc(sizeof(XHostAddress));
    if (host == NULL) {
        *nhosts_return = 0;
        return NULL;
    }
    *nhosts_return = 1;
    host->address = (char *) LOCAL_HOST;
    host->length = strlen(LOCAL_HOST);
    host->family = FamilyInternet;
    return host;
}


int XSetWMHints(Display *display, Window w, XWMHints *wmhints) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetWMHints.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XSetCommand(Display *display, Window w, char **argv, int argc) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-session-manager/XSetCommand.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

void XSetWMNormalHints(Display *display, Window w, XSizeHints *hints) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetWMNormalHints.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XSetClassHint(Display *display, Window w, XClassHint *class_hints) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XSetClassHint.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

Status XStringListToTextProperty(char **list, int count, XTextProperty *text_prop_return) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XStringListToTextProperty.html
    size_t i;
    text_prop_return->format = 8; // STRING
    text_prop_return->encoding = XA_STRING;
    text_prop_return->nitems = 1;
    for (i = 0; i < count; i++) {
        text_prop_return->nitems += strlen(list[i]);
    }
    text_prop_return->value = malloc(sizeof(char) * text_prop_return->nitems);
    if (text_prop_return->value == NULL) {
        text_prop_return->nitems = 0;
        return 0;
    }
    text_prop_return->value[0] = '\0';
    for (i = 0; i < count; i++) {
        strcat((char *) text_prop_return->value, list[i]);
    }
    return 1;
}

void XSetWMClientMachine(Display *display, Window w, XTextProperty *text_prop) {
    // https://tronche.com/gui/x/xlib/ICC/client-to-session-manager/XSetWMClientMachine.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}
