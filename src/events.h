#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <X11/Xlib.h>
#include "SDL.h"

#define SEND_EVENT_CODE 1
#define INTERNAL_EVENT_CODE 2

#define HAS_EVENT_MASK(window, mask) ((GET_WINDOW_STRUCT(window)->eventMask & mask) == mask)

int initEventPipe(Display* display);
unsigned int convertModifierState(Uint16 mod);
Bool postEvent(Display* display, Window eventWindow, unsigned int eventId, ...);
void postExposeEvent(Display* display, Window window, SDL_Rect damagedArea);

#endif /* _EVENTS_H_ */
