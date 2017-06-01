#ifndef VISUAL_H
#define VISUAL_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

//static const Visual STANDARD_VISUAL = {
//    /*ext_data =*/ NULL,
//    /*visualid =*/ TrueColor,
//    /*class =*/ TrueColor,
//    /*red_mask =*/ ((unsigned long) 0xFF) << RED_SHIFT,
//    /*green_mask =*/((unsigned long) 0xFF) << GREEN_SHIFT,
//    /*blue_mask =*/((unsigned long) 0xFF) << BLUE_SHIFT,
//    /*bits_per_rgb =*/8,
//    /*map_entries =*/ INT_MAX,
//};

Bool initVisuals();
void freeVisuals();
Visual* getDefaultVisual(int screenIndex);

#endif //VISUAL_H
