#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <X11/Xlib.h>
#include "SDL.h"

#define SEND_EVENT_CODE 1
#define INTERNAL_EVENT_CODE 2

int initEventPipe(Display* display);
unsigned int convertModifierState(Uint16 mod);
Bool enqueueEvent(Display* dispaly, XEvent* event);

#endif /* _EVENTS_H_ */
