#include "X11/Xlib.h"
#include <errno.h>
#include <fcntl.h>
#include "events.h"
#include "errors.h"
#include "SDL.h"
#include "window.h"
#include "drawing.h"

int eventFds[2];
#define READ_EVENT_FD eventFds[0]
#define WRITE_EVENT_FD eventFds[1]
SDL_Event waitingEvent;
Bool eventWaiting = False;
Bool tmpVar = False;

// TODO: check for errors
#define ENQUEUE_EVENT_IN_PIPE(display) { char buffer = 'e'; write(WRITE_EVENT_FD, &buffer, sizeof(buffer)); (display)->qlen++; }
#define READ_EVENT_IN_PIPE(display) if ((display)->qlen > 0) { char buffer; read(READ_EVENT_FD, &buffer, sizeof(buffer)); (display)->qlen--; }

//TODO: Generate Enter & Leave events on MouseButton down and MouseMotion

void generateExposureEventsForChildren(Display* display, Window window) {
    int i;
    XEvent xEvent;
    Window* children = GET_CHILDREN(window);
    for (i = 0; i < GET_WINDOW_STRUCT(window)->childSpace; i++) {
        if (children[i] == NULL || GET_WINDOW_STRUCT(children[i])->mapState == UnMapped) continue;
        xEvent.xany.serial = 0;
        xEvent.xany.display = display;
        xEvent.xany.send_event = False;
        xEvent.xany.type = Expose;
        xEvent.xany.window = children[i];
        xEvent.type = Expose;
        xEvent.xexpose.type = Expose;
        xEvent.xexpose.serial = 0;
        xEvent.xexpose.send_event = False;
        xEvent.xexpose.display = display;
        xEvent.xexpose.window = children[i];
        xEvent.xexpose.x = 0;
        xEvent.xexpose.y = 0;
        GET_WINDOW_DIMS(children[i], xEvent.xexpose.width, xEvent.xexpose.height);
        xEvent.xexpose.count = 0;
        enqueueEvent(display, &xEvent);
        generateExposureEventsForChildren(display, children[i]);
    }
}

int onSdlEvent(void* userdata, SDL_Event* event) {
    switch (event->type) {
        case SDL_RENDER_TARGETS_RESET:
            generateExposureEventsForChildren((Display *) userdata, SCREEN_WINDOW);
            ENQUEUE_EVENT_IN_PIPE((Display *) userdata);
            break;
        default:
            ENQUEUE_EVENT_IN_PIPE((Display *) userdata);
    }
    return 1;
}

int initEventPipe(Display* display) {
    int flags;
    if (pipe(eventFds) == -1) {
        fprintf(stderr, "Could not create the event pipe: %s", strerror(errno));
        return -1;
    }
    flags = fcntl(READ_EVENT_FD, F_GETFL);
    fcntl(READ_EVENT_FD, F_SETFD, flags | O_NONBLOCK);
    FILE* event_write = fdopen(WRITE_EVENT_FD, "w");
    if (event_write == NULL) {
        fprintf(stderr, "Could not create the write input of the event pipe: %s", strerror(errno));
        return -1;
    }
    FILE* event_read = fdopen(READ_EVENT_FD, "r");
    if (event_read == NULL) {
        fprintf(stderr, "Could not create the read output of the event pipe: %s", strerror(errno));
        return -1;
    }
    int qlen;
    getEventQueueLength(&qlen);
    fprintf(stderr, "Events in queue = %d at initialisation\n", qlen);
    for (; qlen > 0; qlen--) {
        ENQUEUE_EVENT_IN_PIPE(display);
    }
    SDL_AddEventWatch(onSdlEvent, display);
    return READ_EVENT_FD;
}

unsigned int convertModifierState(Uint16 mod) {
    unsigned int state = 0;
    if (HAS_VALUE(mod, KMOD_SHIFT)) {
        state |= ShiftMask;
    }
    if (HAS_VALUE(mod, KMOD_CTRL)) {
        state |= ControlMask;
    }
    if (HAS_VALUE(mod, KMOD_CAPS)) {
        state |= LockMask;
    }
    if (HAS_VALUE(mod, KMOD_NUM)) {
        state |= Mod1Mask;
    }
    return state;
}

int convertEvent(Display* display, SDL_Event* sdlEvent, XEvent* xEvent) {
    Bool sendEvent = False;
    Window eventWindow = None;
    int type = -1;
    XEvent* allocEvent;
    //TODO: this needs to update the window attributes
    switch (sdlEvent->type) {
        case SDL_KEYDOWN:
            type = KeyPress;
            fprintf(stderr, "SDL_KEYDOWN\n");
        case SDL_KEYUP:
            if (sdlEvent->type == SDL_KEYUP) {
                fprintf(stderr, "SDL_KEYUP\n");
                type = KeyRelease;
            }
            xEvent->xkey.type = type;
            xEvent->xkey.serial = 0;
            xEvent->xkey.send_event = sendEvent;
            xEvent->xkey.display = display;
            eventWindow = getWindowFromId(sdlEvent->key.windowID);
            xEvent->xkey.root = eventWindow;
            xEvent->xkey.window = xEvent->xkey.root; // The event window is always the SDL Window.
            if (xEvent->xkey.window == NULL) {
                xEvent->xkey.window = SCREEN_WINDOW;
            }
            xEvent->xkey.subwindow = None;
            xEvent->xkey.time = sdlEvent->key.timestamp;
            SDL_GetMouseState(&xEvent->xkey.x, &xEvent->xkey.y);
            xEvent->xkey.x_root = xEvent->xkey.x; // Because root and window are the same.
            xEvent->xkey.y_root = xEvent->xkey.y;
            xEvent->xkey.state = convertModifierState(sdlEvent->key.keysym.mod);
            xEvent->xkey.keycode = (unsigned int) sdlEvent->key.keysym.scancode;
            xEvent->xkey.same_screen = True;
            break;
        case SDL_MOUSEBUTTONDOWN:
            fprintf(stderr, "SDL_MOUSEBUTTONDOWN\n");
            type = ButtonPress;
            if (!tmpVar) { // TODO: propper implementation
                ENQUEUE_EVENT_IN_PIPE(display);
                eventWaiting = True;
                memcpy(&waitingEvent, sdlEvent, sizeof(SDL_Event));
                type = EnterNotify;
                xEvent->xcrossing.root = getWindowFromId(sdlEvent->button.windowID);
                eventWindow = getContainingWindow(xEvent->xcrossing.root, sdlEvent->button.x, sdlEvent->button.y);
                xEvent->xcrossing.type = type;
                xEvent->xcrossing.serial = 0;
                xEvent->xcrossing.send_event = sendEvent;
                xEvent->xcrossing.display = display;
                xEvent->xcrossing.window = eventWindow;
                // TODO: Check if pointer is in child
                xEvent->xcrossing.subwindow = None;
                xEvent->xcrossing.time = sdlEvent->button.timestamp;
                SDL_GetMouseState(&xEvent->xcrossing.x_root, &xEvent->xcrossing.y_root);
                xEvent->xcrossing.x = xEvent->xcrossing.x_root;
                xEvent->xcrossing.y = xEvent->xcrossing.y_root;
                xEvent->xcrossing.mode = NotifyNormal;
                // TODO: What is this?
                xEvent->xcrossing.detail = NotifyAncestor;
                /*
                 * NotifyAncestor, NotifyVirtual, NotifyInferior,
                 * NotifyNonlinear,NotifyNonlinearVirtual
                 */
                xEvent->xcrossing.same_screen = True;
                xEvent->xcrossing.focus = SDL_GetWindowFlags(SDL_GetWindowFromID(
                        sdlEvent->button.windowID)) & SDL_WINDOW_MOUSE_FOCUS;
                xEvent->xcrossing.state = convertModifierState(SDL_GetModState());
                break;
            }
        case SDL_MOUSEBUTTONUP:
            if (sdlEvent->type == SDL_MOUSEBUTTONUP) {
                fprintf(stderr, "SDL_MOUSEBUTTONUP\n");
                type = ButtonRelease;
            }
            xEvent->xbutton.type = type;
            xEvent->xbutton.serial = 0;
            xEvent->xbutton.send_event = sendEvent;
            xEvent->xbutton.display = display;
            xEvent->xbutton.root = getWindowFromId(sdlEvent->button.windowID);
            if (xEvent->xbutton.root == NULL) {
                xEvent->xbutton.root = SCREEN_WINDOW;
            }
            xEvent->xbutton.subwindow = getContainingWindow(xEvent->xbutton.root, sdlEvent->button.x, sdlEvent->button.y); // The event window is always the SDL Window.
            eventWindow = xEvent->xbutton.subwindow;
//            while (eventWindow != SCREEN_WINDOW && 0 && !HAS_VALUE(GET_WINDOW_STRUCT(eventWindow)->eventMask, ButtonPressMask)) {
//                eventWindow = GET_PARENT(eventWindow);
//            }
            xEvent->xbutton.window = eventWindow;
            xEvent->xbutton.time = sdlEvent->button.timestamp;
            xEvent->xbutton.x = sdlEvent->button.x;
            xEvent->xbutton.y = sdlEvent->button.y;
            xEvent->xbutton.x_root = xEvent->xbutton.x; // Because root and window are the same.
            xEvent->xbutton.y_root = xEvent->xbutton.y;
            xEvent->xbutton.state = convertModifierState(SDL_GetModState());
            if (sdlEvent->button.button == SDL_BUTTON_LEFT) {
                xEvent->xbutton.button = Button1;
            } else if (sdlEvent->button.button == SDL_BUTTON_MIDDLE) {
                xEvent->xbutton.button = Button2;
            } else if (sdlEvent->button.button == SDL_BUTTON_RIGHT) {
                xEvent->xbutton.button = Button3;
            } else if (sdlEvent->button.button == SDL_BUTTON_X1) {
                xEvent->xbutton.button = Button4;
            } else {
                xEvent->xbutton.button = Button5;
            }
            xEvent->xbutton.same_screen = True;
            break;
        case SDL_MOUSEMOTION:
            fprintf(stderr, "SDL_MOUSEMOTION\n");
            type = MotionNotify;
            xEvent->xmotion.type = type;
            xEvent->xmotion.serial = 0;
            xEvent->xmotion.send_event = sendEvent;
            xEvent->xmotion.display = display;
            xEvent->xmotion.root = getWindowFromId(sdlEvent->motion.windowID);
            eventWindow =  getContainingWindow(xEvent->xbutton.root, sdlEvent->motion.x, sdlEvent->motion.y);
            xEvent->xmotion.window = eventWindow; // The event window is always the window the mouse is in.
            if (xEvent->xmotion.window == NULL) {
                xEvent->xmotion.window = SCREEN_WINDOW;
            }
            xEvent->xmotion.subwindow = None;
            xEvent->xmotion.time = sdlEvent->motion.timestamp;
            xEvent->xmotion.x = sdlEvent->motion.x;
            xEvent->xmotion.y = sdlEvent->motion.y;
            xEvent->xmotion.x_root = xEvent->xbutton.x; // Because root and window are the same.
            xEvent->xmotion.y_root = xEvent->xbutton.y;
            xEvent->xmotion.state = convertModifierState(SDL_GetModState());
            xEvent->xmotion.is_hint = NotifyNormal; // TODO: When is this a hint?
            xEvent->xmotion.same_screen = True;
            break;
        case SDL_WINDOWEVENT:
            eventWindow = getWindowFromId(sdlEvent->window.windowID);
            switch (sdlEvent->window.event) {
                case SDL_WINDOWEVENT_SHOWN:
                    fprintf(stderr, "Window %d shown\n", sdlEvent->window.windowID);
                    type = MapNotify;
                    xEvent->xmap.type = type;
                    xEvent->xmap.serial = 0;
                    xEvent->xmap.send_event = sendEvent;
                    xEvent->xmap.display = display;
                    xEvent->xmap.window = eventWindow;
                    xEvent->xmap.event = xEvent->xmap.window;
                    xEvent->xmap.override_redirect = False;
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    fprintf(stderr, "Window %d hidden\n", sdlEvent->window.windowID);
                    type = UnmapNotify;
                    xEvent->xunmap.type = type;
                    xEvent->xunmap.serial = 0;
                    xEvent->xunmap.send_event = sendEvent;
                    xEvent->xunmap.display = display;
                    xEvent->xunmap.window = eventWindow;
                    xEvent->xunmap.event = xEvent->xunmap.window;
                    xEvent->xunmap.from_configure = False;
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    fprintf(stderr, "Window %d exposed\n", sdlEvent->window.windowID);
                    return -1;
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    fprintf(stderr, "Window %d moved to %d,%d\n",
                            sdlEvent->window.windowID, sdlEvent->window.data1,
                            sdlEvent->window.data2);
                case SDL_WINDOWEVENT_RESIZED:
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_RESIZED) {
                        fprintf(stderr, "Window %d resized to %dx%d\n",
                                sdlEvent->window.windowID, sdlEvent->window.data1,
                                sdlEvent->window.data2);
                    }
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        fprintf(stderr, "Window %d size changed to %dx%d\n",
                                sdlEvent->window.windowID, sdlEvent->window.data1,
                                sdlEvent->window.data2);
                    }
                    type = ConfigureNotify;
                    xEvent->xconfigure.type = type;
                    xEvent->xconfigure.serial = 0;
                    xEvent->xconfigure.send_event = sendEvent;
                    xEvent->xconfigure.display = display;
                    xEvent->xconfigure.event = eventWindow;
                    xEvent->xconfigure.window = xEvent->xconfigure.event;
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_MOVED) {
                        xEvent->xconfigure.x = sdlEvent->window.data1;
                        xEvent->xconfigure.y = sdlEvent->window.data2;
                    } else {
                        SDL_GetWindowPosition(SDL_GetWindowFromID(sdlEvent->window.windowID),
                                              &xEvent->xconfigure.x, &xEvent->xconfigure.y);
                    }
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_RESIZED
                        || sdlEvent->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        xEvent->xconfigure.width  = sdlEvent->window.data1;
                        xEvent->xconfigure.height = sdlEvent->window.data2;
                    } else {
                        SDL_GetWindowSize(SDL_GetWindowFromID(sdlEvent->window.windowID),
                                          &xEvent->xconfigure.width, &xEvent->xconfigure.height);
                    }
                    xEvent->xconfigure.border_width;
                    xEvent->xconfigure.above = NULL;
                    xEvent->xconfigure.override_redirect;
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    fprintf(stderr, "Window %d minimized\n", sdlEvent->window.windowID);
                    return -1;
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    fprintf(stderr, "Window %d maximized\n", sdlEvent->window.windowID);
                    return -1;
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    fprintf(stderr, "Window %d restored\n", sdlEvent->window.windowID);
                    return -1;
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    fprintf(stderr, "Mouse entered window %d\n",
                            sdlEvent->window.windowID);
                    type = EnterNotify;
                case SDL_WINDOWEVENT_LEAVE:
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_LEAVE) {
                        fprintf(stderr, "Mouse left window %d\n", sdlEvent->window.windowID);
                        type = LeaveNotify;
                    }
                    xEvent->xcrossing.type = type;
                    xEvent->xcrossing.serial = 0;
                    xEvent->xcrossing.send_event = sendEvent;
                    xEvent->xcrossing.display = display;
                    xEvent->xcrossing.root = eventWindow;
                    xEvent->xcrossing.window = xEvent->xcrossing.root;
                    // TODO: Check if pointer is in child
                    xEvent->xcrossing.subwindow = None;
                    xEvent->xcrossing.time = sdlEvent->window.timestamp;
                    SDL_GetMouseState(&xEvent->xcrossing.x_root, &xEvent->xcrossing.y_root);
                    xEvent->xcrossing.x = xEvent->xcrossing.x_root;
                    xEvent->xcrossing.y = xEvent->xcrossing.y_root;
                    xEvent->xcrossing.mode = NotifyNormal;
                    // TODO: What is this?
                    xEvent->xcrossing.detail = NotifyAncestor;
                    /*
                     * NotifyAncestor, NotifyVirtual, NotifyInferior,
                     * NotifyNonlinear,NotifyNonlinearVirtual
                     */
                    xEvent->xcrossing.same_screen = True;
                    xEvent->xcrossing.focus = SDL_GetWindowFlags(SDL_GetWindowFromID(
                            sdlEvent->window.windowID)) & SDL_WINDOW_MOUSE_FOCUS;
                    xEvent->xcrossing.state = convertModifierState(SDL_GetModState());
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    fprintf(stderr, "Window %d gained keyboard focus\n",
                            sdlEvent->window.windowID);
                    type = FocusIn;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        fprintf(stderr, "Window %d lost keyboard focus\n",
                                sdlEvent->window.windowID);
                        type = FocusOut;
                    }
                    xEvent->xfocus.type = type;
                    xEvent->xfocus.serial = 0;
                    xEvent->xfocus.send_event = sendEvent;
                    xEvent->xfocus.display = display;
                    xEvent->xfocus.window = eventWindow;
                    xEvent->xfocus.mode = NotifyNormal;
                    xEvent->xfocus.detail = NotifyAncestor;
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    fprintf(stderr, "Window %d closed\n", sdlEvent->window.windowID);
                    return -1;
                    break;
                default:
                    fprintf(stderr, "Window %d got unknown event %d\n",
                            sdlEvent->window.windowID, sdlEvent->window.event);
                    return -1;
            }
            break;
        case SDL_QUIT:/**< User-requested quit */
            fprintf(stderr, "SDL_QUIT\n");
            return -1;
        case SDL_APP_TERMINATING: /**< The application is being terminated by the OS
                                     Called on iOS in applicationWillTerminate()
                                     Called on Android in onDestroy()
                                */
            fprintf(stderr, "SDL_APP_TERMINATING\n");
            return -1;
        case SDL_APP_LOWMEMORY: /**< The application is low on memory, free memory if possible.
                                     Called on iOS in applicationDidReceiveMemoryWarning()
                                     Called on Android in onLowMemory()
                                */
            fprintf(stderr, "SDL_APP_LOWMEMORY\n");
            return -1;
        case SDL_APP_WILLENTERBACKGROUND: /**< The application is about to enter the background
                                     Called on iOS in applicationWillResignActive()
                                     Called on Android in onPause()
                                */
            fprintf(stderr, "SDL_APP_WILLENTERBACKGROUND\n");
            return -1;
        case SDL_APP_DIDENTERBACKGROUND: /**< The application did enter the background and may not get CPU for some time
                                     Called on iOS in applicationDidEnterBackground()
                                     Called on Android in onPause()
                                */
            fprintf(stderr, "SDL_APP_DIDENTERBACKGROUND\n");
            return -1;
        case SDL_APP_WILLENTERFOREGROUND: /**< The application is about to enter the foreground
                                     Called on iOS in applicationWillEnterForeground()
                                     Called on Android in onResume()
                                */
            fprintf(stderr, "SDL_APP_WILLENTERFOREGROUND\n");
            return -1;
        case SDL_APP_DIDENTERFOREGROUND: /**< The application is now interactive
                                     Called on iOS in applicationDidBecomeActive()
                                     Called on Android in onResume()
                                */
            fprintf(stderr, "SDL_APP_DIDENTERFOREGROUND\n");
            return -1;
        case SDL_SYSWMEVENT: /**< System specific event */
            fprintf(stderr, "SDL_SYSWMEVENT\n");
            return -1;
        case SDL_TEXTEDITING:            /**< Keyboard text editing (composition) */
            fprintf(stderr, "SDL_TEXTEDITING\n");
            return -1;
        case SDL_TEXTINPUT:              /**< Keyboard text input */
            fprintf(stderr, "SDL_TEXTINPUT\n");
            return -1;
        case SDL_MOUSEWHEEL:             /**< Mouse wheel motion */
            fprintf(stderr, "SDL_MOUSEWHEEL\n");
            return -1;
        case SDL_JOYAXISMOTION: /**< Joystick axis motion */
            fprintf(stderr, "SDL_JOYAXISMOTION\n");
            return -1;
        case SDL_JOYBALLMOTION:          /**< Joystick trackball motion */
            fprintf(stderr, "SDL_JOYBALLMOTION\n");
            return -1;
        case SDL_JOYHATMOTION:           /**< Joystick hat position change */
            fprintf(stderr, "SDL_JOYHATMOTION\n");
            return -1;
        case SDL_JOYBUTTONDOWN:          /**< Joystick button pressed */
            fprintf(stderr, "SDL_JOYBUTTONDOWN\n");
            return -1;
        case SDL_JOYBUTTONUP:            /**< Joystick button released */
            fprintf(stderr, "SDL_JOYBUTTONUP\n");
            return -1;
        case SDL_JOYDEVICEADDED:         /**< A new joystick has been inserted into the system */
            fprintf(stderr, "SDL_JOYDEVICEADDED\n");
            return -1;
        case SDL_JOYDEVICEREMOVED:       /**< An opened joystick has been removed */
            fprintf(stderr, "SDL_JOYDEVICEREMOVED\n");
            return -1;
        case SDL_CONTROLLERAXISMOTION: /**< Game controller axis motion */
            fprintf(stderr, "SDL_CONTROLLERAXISMOTION\n");
            return -1;
        case SDL_CONTROLLERBUTTONDOWN:          /**< Game controller button pressed */
            fprintf(stderr, "SDL_CONTROLLERBUTTONDOWN\n");
            return -1;
        case SDL_CONTROLLERBUTTONUP:            /**< Game controller button released */
            fprintf(stderr, "SDL_CONTROLLERBUTTONUP\n");
            return -1;
        case SDL_CONTROLLERDEVICEADDED:         /**< A new Game controller has been inserted into the system */
            fprintf(stderr, "SDL_CONTROLLERDEVICEADDED\n");
            return -1;
        case SDL_CONTROLLERDEVICEREMOVED:       /**< An opened Game controller has been removed */
            fprintf(stderr, "SDL_CONTROLLERDEVICEREMOVED\n");
            return -1;
        case SDL_CONTROLLERDEVICEREMAPPED:      /**< The controller mapping was updated */
            fprintf(stderr, "SDL_CONTROLLERDEVICEREMAPPED\n");
            return -1;
        case SDL_FINGERDOWN: // Should not happen
            fprintf(stderr, "FINGER BUTTONDOWN\n");
            return -1;
            type = ButtonPress;
        case SDL_FINGERUP: // Should not happen
            fprintf(stderr, "SDL_FINGERUP\n");
            return -1;
            if (sdlEvent->type == SDL_FINGERUP) {
                type = ButtonRelease;
            }
            xEvent->xbutton.type = type;
            xEvent->xbutton.serial = 0;
            xEvent->xbutton.send_event = sendEvent;
            xEvent->xbutton.display = display;
            eventWindow = SCREEN_WINDOW;
            xEvent->xbutton.root = eventWindow;
            xEvent->xbutton.window = xEvent->xbutton.root; // The event window is always the SDL Window.
            xEvent->xbutton.subwindow = None;
            xEvent->xbutton.time = sdlEvent->tfinger.timestamp;
            xEvent->xbutton.x = sdlEvent->tfinger.x;
            xEvent->xbutton.y = sdlEvent->tfinger.y;
            xEvent->xbutton.x_root = xEvent->xbutton.x; // Because root and window are the same.
            xEvent->xbutton.y_root = xEvent->xbutton.y;
            xEvent->xbutton.state = convertModifierState(SDL_GetModState());
            xEvent->xbutton.button = Button1;
            xEvent->xbutton.same_screen = True;
            break;
        case SDL_FINGERMOTION: // Should not happen
            fprintf(stderr, "SDL_FINGERMOTION\n");
            return -1;
            type = MotionNotify;
            xEvent->xmotion.type = type;
            xEvent->xmotion.serial = 0;
            xEvent->xmotion.send_event = sendEvent;
            xEvent->xmotion.display = display;
            eventWindow = SCREEN_WINDOW;
            xEvent->xmotion.root = eventWindow;
            xEvent->xmotion.window = xEvent->xmotion.root; // The event window is always the SDL Window.
            xEvent->xmotion.subwindow = None;
            xEvent->xmotion.time = sdlEvent->tfinger.timestamp;
            xEvent->xmotion.x = sdlEvent->tfinger.x;
            xEvent->xmotion.y = sdlEvent->tfinger.y;
            xEvent->xmotion.x_root = xEvent->xbutton.x; // Because root and window are the same.
            xEvent->xmotion.y_root = xEvent->xbutton.y;
            xEvent->xmotion.state = convertModifierState(SDL_GetModState());
            xEvent->xmotion.is_hint = NotifyNormal; // TODO: When is this a hint?
            xEvent->xmotion.same_screen = True;
            break;
        case SDL_DOLLARGESTURE:
            fprintf(stderr, "SDL_DOLLARGESTURE\n");
            return -1;
        case SDL_DOLLARRECORD:
            fprintf(stderr, "SDL_DOLLARRECORD\n");
            return -1;
        case SDL_MULTIGESTURE:
            fprintf(stderr, "SDL_MULTIGESTURE\n");
            return -1;
        case SDL_CLIPBOARDUPDATE: /**< The clipboard changed */
            fprintf(stderr, "SDL_CLIPBOARDUPDATE\n");
            return -1;
        case SDL_DROPFILE: /**< The system requests a file open */
            fprintf(stderr, "SDL_DROPFILE\n");
            return -1;
        case SDL_RENDER_TARGETS_RESET:
            fprintf(stderr, "SDL_RENDER_TARGETS_RESET\n");
            type = Expose;
            eventWindow = *GET_CHILDREN(SCREEN_WINDOW);
            xEvent->xexpose.type = type;
            xEvent->xexpose.serial = 0;
            xEvent->xexpose.send_event = False;
            xEvent->xexpose.display = display;
            xEvent->xexpose.window = eventWindow;
            GET_WINDOW_POS(eventWindow, xEvent->xexpose.x, xEvent->xexpose.y);
            GET_WINDOW_DIMS(eventWindow, xEvent->xexpose.width, xEvent->xexpose.height);
            xEvent->xexpose.count = 0;
        default:
            if (sdlEvent->type >= SDL_USEREVENT && sdlEvent->type <= SDL_LASTEVENT) {
                if (sdlEvent->user.code == SEND_EVENT_CODE) {
                    allocEvent = sdlEvent->user.data1;
                    memcpy(xEvent, allocEvent, sizeof(XEvent));
                    fprintf(stderr, "SDL_USEREVENT: %d\n", xEvent->type);
                    free(allocEvent);
                    return 0;
                }
            }
            return -1;
    }
    xEvent->xany.serial = 0;
    xEvent->xany.display = display;
    xEvent->xany.send_event = sendEvent;
    xEvent->xany.type = type;
    xEvent->xany.window = eventWindow;
    xEvent->type = type;
    return 0;
}

void XNextEvent(Display* display, XEvent* event_return) {
    // https://tronche.com/gui/x/xlib/event-handling/manipulating-event-queue/XNextEvent.html
    SDL_Event event;
    Bool done = False;
    while (!done) {
        int qlen;
        getEventQueueLength(&qlen);
        fprintf(stderr, "Events in queue = %d, qlen = %d\n", qlen, display->qlen);
        if (qlen == 0 || eventWaiting || SDL_WaitEvent(&event) == 1) {
            tmpVar = False;
            if (eventWaiting) {
                event = waitingEvent;
                eventWaiting = False;
                tmpVar = True;
            }
            // Clear the event from the pipe;
            READ_EVENT_IN_PIPE(display);
            if (convertEvent(display, &event, event_return) == 0) {
                done = True;
            } else {
                printWindowHierarchy();
                fprintf(stderr, "Got unknown SDL event %d!\n", event.type);
                event_return->type = Expose;
                event_return->xany.serial = 0;
                event_return->xany.display = display;
                event_return->xany.send_event = False;
                event_return->xany.type = Expose;
                event_return->xany.window = *GET_CHILDREN(display->screens[0].root);
                event_return->xexpose.type = Expose;
                event_return->xexpose.serial = 0;
                event_return->xexpose.send_event = False;
                event_return->xexpose.display = display;
                event_return->xexpose.window = event_return->xany.window;
                event_return->xexpose.x = 0;
                event_return->xexpose.y = 0;
                event_return->xexpose.width = 0;
                event_return->xexpose.height = 0;
                event_return->xexpose.count = 0;
                done = True;
            }
        } else {
            fprintf(stderr, "SDL_WaitEvent failed: %s, retrying...\n", SDL_GetError());
        }
        fflush(stderr);
    }
    fprintf(stderr, "Leaving XNextEvent\n");
}

Bool enqueueEvent(Display* display, XEvent* event) {
    XEvent* allocEvent = malloc(sizeof(XEvent));
    if (allocEvent == NULL) {
        fprintf(stderr, "Failed to enqueue event: Could not allocate XEvent!\n");
        return False;
    }
    static Uint32 sendEventType = (Uint32) -1;
    if (sendEventType == ((Uint32) -1)) {
        sendEventType = SDL_RegisterEvents(1);
    }
    if (sendEventType != ((Uint32) -1)) {
        memcpy(allocEvent, event, sizeof(XEvent));
        if (event->type == MapNotify) {
            fprintf(stderr, "Mapping window %p\n", event->xmap.window);
        } else if (event->type == MapRequest) {
            fprintf(stderr, "Mapping window %p\n", event->xmaprequest.window);
        }
        SDL_Event sdlEvent;
        SDL_zero(sdlEvent);
        sdlEvent.type = sendEventType;
        sdlEvent.user.code = SEND_EVENT_CODE;
        sdlEvent.user.data1 = allocEvent;
        fprintf(stderr, "Enqueuing event\n");
        SDL_PushEvent(&sdlEvent);
        return True;
    }
    fprintf(stderr, "Failed to send event: SDL_RegisterEvents failed!");
    free(allocEvent);
    return False;
}

Status XSendEvent(Display* display, Window window, Bool propagate, long event_mask, XEvent* event_send) {
    // https://tronche.com/gui/x/xlib/event-handling/XSendEvent.html
    //  We have to assume that window is our window
    TYPE_CHECK(window, WINDOW, XCB_SEND_EVENT, display, 0);
    event_send->xany.send_event = True;
    return enqueueEvent(display, event_send) ? 1 : 0;
}

void XSelectInput(Display *display, Window w, long event_mask) {
    // https://tronche.com/gui/x/xlib/event-handling/XSelectInput.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

Bool XFilterEvent(XEvent *event, Window w) {
    // http://www.x.org/archive/X11R7.6/doc/man/man3/XFilterEvent.3.xhtml
    // We dont get an event from sdl, if an IM gets it before us.
    return False;
}

Bool getEventQueueLength(int* qlen) {
    SDL_Event tmp[25];
    *qlen = SDL_PeepEvents((SDL_Event*) &tmp, 25, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
    if (*qlen < 0) {
        fprintf(stderr, "Failed to get the length of the input queue: %s\n", SDL_GetError());
        return False;
    }
    return True;
}

int XEventsQueued(Display *display, int mode) {
    // https://tronche.com/gui/x/xlib/event-handling/XEventsQueued.html
    if (display->qlen == 0 && mode != QueuedAlready) {
        SDL_PumpEvents();
    }
    return display->qlen;
}

void XFlush(Display *display) {
    // https://tronche.com/gui/x/xlib/event-handling/XFlush.html
//    SDL_PumpEvents(); // TODO: This locks up the main thread
}
