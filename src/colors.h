#ifndef _COLORS_H_
#define _COLORS_H_

#include "SDL.h"

#if SDL_BYTEORDER != SDL_BIG_ENDIAN
#  define RED_SHIFT   24
#  define GREEN_SHIFT 16
#  define BLUE_SHIFT  8
#  define ALPHA_SHIFT 0
#else
#  define RED_SHIFT   0
#  define GREEN_SHIFT 8
#  define BLUE_SHIFT  16
#  define ALPHA_SHIFT 24
#endif

#define GET_RED_FROM_COLOR(color)   ((Uint8) ((color >> RED_SHIFT)   & 0xFF))
#define GET_GREEN_FROM_COLOR(color) ((Uint8) ((color >> GREEN_SHIFT) & 0xFF))
#define GET_BLUE_FROM_COLOR(color)  ((Uint8) ((color >> BLUE_SHIFT)  & 0xFF))
#define GET_ALPHA_FROM_COLOR(color) ((Uint8) ((color >> ALPHA_SHIFT) & 0xFF))

// TODO: Make these to real XIDs

#define GREY_SCALE_COLORMAP ((XID) 1)
#define REAL_COLOR_COLORMAP ((XID) 2)

SDL_Color uLongToColor(SDL_PixelFormat* pixelFormat, unsigned long color);

#endif /* _COLORS_H_ */
