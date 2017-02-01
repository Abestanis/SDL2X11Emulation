
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "resourceTypes.h"

#define GET_DISPLAY(display) ((_XPrivDisplay) (display))
#define SET_X_SERVER_REQUEST(display, requestId) GET_DISPLAY(display)->request = requestId

#endif //_DISPLAY_H
