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
    long foreground;
    long background;
    short fillStyle;
    Pixmap stipple;
    Font font;
} GraphicContext;

#define GET_GC(gc) GET_GC_FROM_XID(((struct _XGC*) (gc))->gid)
#define GET_GC_FROM_XID(id) ((GraphicContext*) GET_XID_VALUE(id))

#endif /* GC_H */
