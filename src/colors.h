#ifndef _COLORS_H_
#define _COLORS_H_

#include "SDL.h"

#define GREY_SCALE_COLORMAP ((XID) 1)
#define REAL_COLOR_COLORMAP ((XID) 2)

SDL_Color uLongToColor(SDL_PixelFormat* pixelFormat, unsigned long color);

#endif /* _COLORS_H_ */
