#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "SDL.h"

#define SEND_EVENT_CODE 1

int initEventPipe(Display* display);
unsigned int convertModifierState(Uint16 mod);
Bool enqueueEvent(Display* dispaly, XEvent* event);

#endif /* _EVENTS_H_ */
