#ifndef _ERRORS_H_
#define _ERRORS_H_

#include "X11/Xlib.h"
#include "X11/Xproto.h"
#include "X11/Xutil.h"


int default_error_handler(Display* display, XErrorEvent* event);
void handleError(int type, Display* display, XID resourceId, unsigned long serial,
                 unsigned char error_code, unsigned char minor_code);
void handleOutOfMemory(int type, Display* display, unsigned long serial, unsigned char minor_code);
unsigned char resourceTypeToErrorCode(XResourceType resourceType);

#endif /* _ERRORS_H_ */
