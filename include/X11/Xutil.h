#ifndef _XUTIL_H_
#define _XUTIL_H_

/*
 * return codes for XReadBitmapFile and XWriteBitmapFile
 */
#define BitmapSuccess		0
#define BitmapOpenFailed 	1
#define BitmapFileInvalid 	2
#define BitmapNoMemory		3

#include "Xregion.h"
/*
 * opaque reference to Region data type
 */
//typedef struct _XRegion *Region;
typedef struct pixman_region16* Region;

/* Return values from XRectInRegion() */

#define RectangleOut  0
#define RectangleIn   1
#define RectanglePart 2

/*
 * Information used by the visual utility routines to find desired visual
 * type from the many visuals a display may support.
 */

typedef struct {
    Visual *visual;
    VisualID visualid;
    int screen;
    int depth;
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;					/* C++ */
#else
    int class;
#endif
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
    int colormap_size;
    int bits_per_rgb;
} XVisualInfo;

#define VisualNoMask           0x0
#define VisualIDMask           0x1
#define VisualScreenMask       0x2
#define VisualDepthMask        0x4
#define VisualClassMask        0x8
#define VisualRedMaskMask      0x10
#define VisualGreenMaskMask    0x20
#define VisualBlueMaskMask     0x40
#define VisualColormapSizeMask 0x80
#define VisualBitsPerRGBMask   0x100
#define VisualAllMask          0x1FF

/*
 * The next block of definitions are for window manager properties that
 * clients and applications use for communication.
 */

/* flags argument in size hints */
#define USPosition	(1L << 0) /* user specified x, y */
#define USSize		(1L << 1) /* user specified width, height */

#define PPosition	(1L << 2) /* program specified position */
#define PSize		(1L << 3) /* program specified size */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */
#define PResizeInc	(1L << 6) /* program specified resize increments */
#define PAspect		(1L << 7) /* program specified min and max aspect ratios */
#define PBaseSize	(1L << 8) /* program specified base for incrementing */
#define PWinGravity	(1L << 9) /* program specified window gravity */

typedef struct {
    char *res_name;
    char *res_class;
} XClassHint;

/*
 * new version containing base_width, base_height, and win_gravity fields;
 * used with WM_NORMAL_HINTS.
 */
typedef struct {
    long flags;	/* marks which fields in this structure are defined */
//    int x, y;		/* obsolete for new window mgrs, but clients */
//    int width, height;	/* should set so old wm's don't mess up */
    int min_width, min_height;
    int max_width, max_height;
    int width_inc, height_inc;
    struct {
        int x;	/* numerator */
        int y;	/* denominator */
    } min_aspect, max_aspect;
    int base_width, base_height;		/* added by ICCCM version 1 */
    int win_gravity;			/* added by ICCCM version 1 */
} XSizeHints;

typedef struct {
    long flags;	/* marks which fields in this structure are defined */
    Bool input;	/* does this application rely on the window manager to
			get keyboard input? */
    int initial_state;	/* see below */
    Pixmap icon_pixmap;	/* pixmap to be used as icon */
    Window icon_window; 	/* window to be used as icon */
    int icon_x, icon_y; 	/* initial position of icon */
    Pixmap icon_mask;	/* icon mask bitmap */
    XID window_group;	/* id of related window group */
    /* this structure may be extended in the future */
} XWMHints;

/* definition for flags of XWMHints */

#define InputHint 		(1L << 0)
#define StateHint 		(1L << 1)
#define IconPixmapHint		(1L << 2)
#define IconWindowHint		(1L << 3)
#define IconPositionHint 	(1L << 4)
#define IconMaskHint		(1L << 5)
#define WindowGroupHint		(1L << 6)
#define AllHints (InputHint|StateHint|IconPixmapHint|IconWindowHint| \
IconPositionHint|IconMaskHint|WindowGroupHint)

/* definitions for initial window state */
#define WithdrawnState 0	/* for windows that are not mapped */
#define NormalState 1	/* most applications want to start this way */
#define IconicState 3	/* application wants to start as an icon */


/*
 * new structure for manipulating TEXT properties; used with WM_NAME,
 * WM_ICON_NAME, WM_CLIENT_MACHINE, and WM_COMMAND.
 */
typedef struct {
    unsigned char *value;		/* same as Property routines */
    Atom encoding;			/* prop type */
    int format;				/* prop data format: 8, 16, or 32 */
    unsigned long nitems;		/* number of data items in value */
} XTextProperty;

/*
 * Compose sequence status structure, used in calling XLookupString.
 */
typedef struct _XComposeStatus {
//    XPointer compose_ptr;	/* state table pointer */
//    int chars_matched;		/* match state */
} XComposeStatus;

extern void XClipBox(
#if NeedFunctionPrototypes
        Region		/* r */,
        XRectangle*		/* rect_return */
#endif
);

extern Region XCreateRegion(
#if NeedFunctionPrototypes
        void
#endif
);

extern void XDestroyRegion(
#if NeedFunctionPrototypes
        Region		/* r */
#endif
);

extern void XIntersectRegion(
#if NeedFunctionPrototypes
        Region		/* sra */,
        Region		/* srb */,
        Region		/* dr_return */
#endif
);

extern int XRectInRegion(
#if NeedFunctionPrototypes
        Region		/* r */,
        int			/* x */,
        int			/* y */,
        unsigned int	/* width */,
        unsigned int	/* height */
#endif
);

extern void XSetRegion(
#if NeedFunctionPrototypes
        Display*		/* display */,
        GC			/* gc */,
        Region		/* r */
#endif
);

extern void XUnionRectWithRegion(
#if NeedFunctionPrototypes
        XRectangle*		/* rectangle */,
        Region		/* src_region */,
        Region		/* dest_region_return */
#endif
);

extern void XSubtractRegion(
#if NeedFunctionPrototypes
        Region		/* sra */,
        Region		/* srb */,
        Region		/* dr_return */
#endif
);

#endif /* _XUTIL_H_ */
