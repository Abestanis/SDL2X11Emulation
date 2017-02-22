#ifndef GC_H
#define GC_H

#include "X11/Xlib.h"
#include "resourceTypes.h"

struct _XGC {
    XExtData *ext_data;	/* hook for extension to hang data */
    GContext gid;	/* protocol ID for graphics context */
};

typedef struct _GraphicContext {
    int lineWidth;
    unsigned long foreground;
    unsigned long background;
    int fillStyle;
    Pixmap stipple;
    Font font;
    int function;
    unsigned long planeMask;
    int lineStyle;
    int capStyle;
    int joinStyle;
    int fillRule;
    Pixmap tile;
    int tileStipOriginX;
    int tileStipOriginY;
    int subWindowMode;
    int graphicsExposures;
    int clipOriginX;
    int clipOriginY;
    Pixmap clipMask;
    int dashOffset;
    char* dashes; // If numDashes is uneven, this has to be treated as concatenated with itself.
    size_t numDashes;
    int arcMode;
} GraphicContext;

#define GET_GC(gc) GET_GC_FROM_XID(((struct _XGC*) (gc))->gid)
#define GET_GC_FROM_XID(id) ((GraphicContext*) GET_XID_VALUE(id))

#endif /* GC_H */
