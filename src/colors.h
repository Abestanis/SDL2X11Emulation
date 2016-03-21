#ifndef _COLORS_H_
#define _COLORS_H_

#include "SDL.h"

#define GREY_SCALE_COLORMAP (unsigned int*) 0
#define REAL_COLOR_COLORMAP (unsigned int*) 1

SDL_Color uLongToColor(SDL_PixelFormat* pixelFormat, unsigned long color);

#endif /* _COLORS_H_ */
