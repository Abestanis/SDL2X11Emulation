#ifndef XLIB_H
#define XLIB_H

#include <X11/X.h>
#include <X11/Xproto.h>

#define Bool int
#define True  1
#define False 0

#define XMaxTransChars 4

typedef int Status;

#define QueuedAlready 0
#define QueuedAfterReading 1
#define QueuedAfterFlush 2

#define ConnectionNumber(dpy) 	((dpy)->fd)
#define RootWindow(dpy, scr) 	(((dpy)->screens[(scr)]).root)
#define DefaultScreen(dpy) 	((dpy)->default_screen)
//#define DefaultRootWindow(dpy) 	(((dpy)->screens[(dpy)->default_screen]).root)
#define DefaultVisual(dpy, scr) (((dpy)->screens[(scr)]).root_visual)
//#define DefaultGC(dpy, scr) 	(((dpy)->screens[(scr)]).default_gc)
#define BlackPixel(dpy, scr) 	(((dpy)->screens[(scr)]).black_pixel)
#define WhitePixel(dpy, scr) 	(((dpy)->screens[(scr)]).white_pixel)
#define AllPlanes 		((unsigned long)~0L)
#define QLength(dpy) 		((dpy)->qlen)
#define DisplayWidth(dpy, scr) 	(((dpy)->screens[(scr)]).width)
#define DisplayHeight(dpy, scr) (((dpy)->screens[(scr)]).height)
//#define DisplayWidthMM(dpy, scr)(((dpy)->screens[(scr)]).mwidth)
//#define DisplayHeightMM(dpy, scr)(((dpy)->screens[(scr)]).mheight)
//#define DisplayPlanes(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
//#define DisplayCells(dpy, scr) 	(DefaultVisual((dpy), (scr))->map_entries)
#define ScreenCount(dpy) 	((dpy)->nscreens)
#define ServerVendor(dpy) 	((dpy)->vendor)
#define ProtocolVersion(dpy) 	((dpy)->proto_major_version)
#define ProtocolRevision(dpy) 	((dpy)->proto_minor_version)
#define VendorRelease(dpy) 	((dpy)->release)
//#define DisplayString(dpy) 	((dpy)->display_name)
#define DefaultDepth(dpy, scr) 	(((dpy)->screens[(scr)]).root_depth)
#define DefaultColormap(dpy, scr)(((dpy)->screens[(scr)]).cmap)
//#define BitmapUnit(dpy) 	((dpy)->bitmap_unit)
//#define BitmapBitOrder(dpy) 	((dpy)->bitmap_bit_order)
//#define BitmapPad(dpy) 		((dpy)->bitmap_pad)
//#define ImageByteOrder(dpy) 	((dpy)->byte_order)
#define NextRequest(dpy)	((dpy)->request + 1)
#define LastKnownRequestProcessed(dpy)	((dpy)->request)

/* macros for screen oriented applications (toolkit) */
#define ScreenOfDisplay(dpy, scr)(&((dpy)->screens[(scr)]))
#define DefaultScreenOfDisplay(dpy) (&((dpy)->screens[(dpy)->default_screen]))
#define DisplayOfScreen(s)	((s)->display)
#define RootWindowOfScreen(s)	((s)->root)
#define BlackPixelOfScreen(s)	((s)->black_pixel)
#define WhitePixelOfScreen(s)	((s)->white_pixel)
#define DefaultColormapOfScreen(s)((s)->cmap)
#define DefaultDepthOfScreen(s)	((s)->root_depth)
//#define DefaultGCOfScreen(s)	((s)->default_gc)
#define DefaultVisualOfScreen(s)((s)->root_visual)
#define WidthOfScreen(s)	((s)->width)
#define HeightOfScreen(s)	((s)->height)
#define WidthMMOfScreen(s)	((s)->mwidth)
#define HeightMMOfScreen(s)	((s)->mheight)
//#define PlanesOfScreen(s)	((s)->root_depth)
#define CellsOfScreen(s)	(DefaultVisualOfScreen((s))->map_entries)
//#define MinCmapsOfScreen(s)	((s)->min_maps)
//#define MaxCmapsOfScreen(s)	((s)->max_maps)
//#define DoesSaveUnders(s)	((s)->save_unders)
//#define DoesBackingStore(s)	((s)->backing_store)
//#define EventMaskOfScreen(s)	((s)->root_input_mask)

// ---------------------------------------------------------------------------------------- //

/*
 * Data structure for host setting; getting routines.
 *
 */

typedef struct {
    int family;		/* for example FamilyInternet */
    int length;		/* length of address, in bytes */
    char *address;		/* pointer to where to find the bytes */
} XHostAddress;

/*
 * Visual structure; contains information about colormapping possible.
 */
typedef struct {
//    XExtData *ext_data;	/* hook for extension to hang data */
    VisualID visualid;	/* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;		/* C++ class of screen (monochrome, etc.) */
#else
    int class;		/* class of screen (monochrome, etc.) */
#endif
    unsigned long red_mask, green_mask, blue_mask;	/* mask values */
    int bits_per_rgb;	/* log base 2 of distinct color values */
    int map_entries;	/* color map entries */
} Visual;

/*
 * Data structure for XReconfigureWindow
 */
typedef struct {
    int x, y;
    int width, height;
    int border_width;
    Window sibling;
    int stack_mode;
} XWindowChanges;

typedef struct {
    Pixmap background_pixmap;	/* background or None or ParentRelative */
    unsigned long background_pixel;	/* background pixel */
    Pixmap border_pixmap;	/* border of the window */
    unsigned long border_pixel;	/* border pixel value */
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preseved if possible */
    unsigned long backing_pixel;/* value to use in restoring planes */
    Bool save_under;		/* should bits under be saved? (popups) */
    long event_mask;		/* set of events that should be saved */
    long do_not_propagate_mask;	/* set of events that should not propagate */
    Bool override_redirect;	/* boolean value for override-redirect */
    Colormap colormap;		/* color map to be associated with window */
    Cursor cursor;		/* cursor to be displayed (or None) */
} XSetWindowAttributes;

typedef struct {
    int x, y;			/* location of window */
    int width, height;		/* width and height of window */
//    int border_width;		/* border width of window */
    int depth;          	/* depth of window */
    Visual *visual;		/* the associated visual structure */
    Window root;        	/* root of screen containing window */
//#if defined(__cplusplus) || defined(c_plusplus)
//    int c_class;		/* C++ InputOutput, InputOnly*/
//#else
//    int class;			/* InputOutput, InputOnly*/
//#endif
//    int bit_gravity;		/* one of bit gravity values */
//    int win_gravity;		/* one of the window gravity values */
//    int backing_store;		/* NotUseful, WhenMapped, Always */
//    unsigned long backing_planes;/* planes to be preserved if possible */
//    unsigned long backing_pixel;/* value to be used when restoring planes */
//    Bool save_under;		/* boolean, should bits under be saved? */
    Colormap colormap;		/* color map to be associated with window */
//    Bool map_installed;		/* boolean, is color map currently installed*/
    int map_state;		/* IsUnmapped, IsUnviewable, IsViewable */
//    long all_event_masks;	/* set of events all people have interest in*/
//    long your_event_mask;	/* my event mask */
//    long do_not_propagate_mask; /* set of events that should not propagate */
//    Bool override_redirect;	/* boolean value for override-redirect */
//    Screen *screen;		/* back pointer to correct screen */
} XWindowAttributes;

typedef struct {
    short x, y;
} XPoint;

/*
 * Data structure used by color operations
 */
typedef struct {
    unsigned long pixel;
    unsigned short red, green, blue;
    char flags;  /* do_red, do_green, do_blue */
    char pad;
} XColor;

/*
 * Data structures for graphics operations.  On most machines, these are
 * congruent with the wire protocol structures, so reformatting the data
 * can be avoided on these architectures.
 */
typedef struct {
    short x1, y1, x2, y2;
} XSegment;

/*
 * Data structure for setting graphics context.
 */
typedef struct {
    int function;		/* logical operation */
    unsigned long plane_mask;/* plane mask */
    unsigned long foreground;/* foreground pixel */
    unsigned long background;/* background pixel */
    int line_width;		/* line width */
    int line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
    int cap_style;	  	/* CapNotLast, CapButt,
				   CapRound, CapProjecting */
    int join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
    int fill_style;	 	/* FillSolid, FillTiled,
				   FillStippled, FillOpaeueStippled */
    int fill_rule;	  	/* EvenOddRule, WindingRule */
    int arc_mode;		/* ArcChord, ArcPieSlice */
    Pixmap tile;		/* tile pixmap for tiling operations */
    Pixmap stipple;		/* stipple 1 plane pixmap for stipping */
    int ts_x_origin;	/* offset for tile or stipple operations */
    int ts_y_origin;
    Font font;	        /* default text font for text operations */
    int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
    Bool graphics_exposures;/* boolean, should exposures be generated */
    int clip_x_origin;	/* origin for clipping */
    int clip_y_origin;
    Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
    int dash_offset;	/* patterned/dashed line information */
    char dashes;
} XGCValues;

/*
 * Graphics context.  The contents of this structure are implementation
 * dependent.  A GC should be treated as opaque by application code.
 */

typedef XGCValues *GC;

typedef struct {
    short x, y;
    unsigned short width, height;
} XRectangle;

/*
 * Data structure for "image" data, used by image manipulation routines.
 */

typedef struct _XImage {
    int width, height;		/* size of image */
//    int xoffset;		/* number of pixels offset in X direction */
    int format;			/* XYBitmap, XYPixmap, ZPixmap */
    char *data;			/* pointer to image data */
    int byte_order;		/* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;		/* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;	/* LSBFirst, MSBFirst */
//    int bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */
    int depth;			/* depth of image */
    int bytes_per_line;		/* accelarator to next line */
    int bits_per_pixel;		/* bits per pixel (ZPixmap) */
//    unsigned long red_mask;	/* bits in z arrangment */
//    unsigned long green_mask;
//    unsigned long blue_mask;
//    XPointer obdata;		/* hook for the object routines to hang on */
//    struct funcs {		/* image manipulation routines */
//        struct _XImage *(*create_image)();
//#if NeedFunctionPrototypes
//        int (*destroy_image)        (struct _XImage *);
//        unsigned long (*get_pixel)  (struct _XImage *, int, int);
//        int (*put_pixel)            (struct _XImage *, int, int, unsigned long);
//        struct _XImage *(*sub_image)(struct _XImage *, int, int, unsigned int, unsigned int);
//        int (*add_pixel)            (struct _XImage *, long);
//#else
//        int (*destroy_image)();
//        unsigned long (*get_pixel)();
//        int (*put_pixel)();
//        struct _XImage *(*sub_image)();
//        int (*add_pixel)();
//#endif
//    } f;
} XImage;

typedef struct {
////    XExtData *ext_data;	/* hook for extension to hang data */
    struct _XDisplay *display;/* back pointer to display structure */
    Window root;		/* Root window id. */
    int width, height;	/* width and height of screen */
    int mwidth, mheight;	/* width and height of  in millimeters */
//    int ndepths;		/* number of depths possible */
////    Depth *depths;		/* list of allowable depths on the screen */
    int root_depth;		/* bits per pixel */
    Visual *root_visual;	/* root visual */
//    GC default_gc;		/* GC for the root root visual */
    Colormap cmap;		/* default color map */
    unsigned long white_pixel;
    unsigned long black_pixel;	/* White and Black pixel values */
//    int max_maps, min_maps;	/* max and min color maps */
//    int backing_store;	/* Never, WhenMapped, Always */
//    Bool save_unders;
//    long root_input_mask;	/* initial root input mask */
} Screen;

//typedef struct _XTime {
//
//} Time;

//typedef struct _XTime {
//
//} Time;

//typedef struct _XTime {
//
//} Time;

//typedef struct _XTime {
//
//} Time;

/*
 * Display datatype maintaining display specific data.
 * The contents of this structure are implementation dependent.
 * A Display should be treated as opaque by application code.
 */
typedef struct _XDisplay {
//    XExtData *ext_data;	/* hook for extension to hang data */
//    struct _XFreeFuncs *free_funcs; /* internal free functions */
    int fd;			/* Network socket. */
//    int conn_checker;         /* ugly thing used by _XEventsQueued */
    int proto_major_version;/* maj. version of server's X protocol */
    int proto_minor_version;/* minor version of servers X protocol */
    char *vendor;		/* vendor of the server hardware */
//    XID resource_base;	/* resource ID base */
//    XID resource_mask;	/* resource ID mask bits */
//    XID resource_id;	/* allocator current ID */
//    int resource_shift;	/* allocator shift to correct bits */
//    XID (*resource_alloc)(); /* allocator function */
    int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
//    int bitmap_unit;	/* padding and data requirements */
//    int bitmap_pad;		/* padding requirements on bitmaps */
//    int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
//    int nformats;		/* number of pixmap formats in list */
//    ScreenFormat *pixmap_format;	/* pixmap format list */
//    int vnumber;		/* Xlib's X protocol version number. */
    int release;		/* release of the server */
//    struct _XSQEvent *head, *tail;	/* Input event queue. */
    int qlen;		/* Length of input event queue */
    unsigned long request;	/* sequence number of last request. */
//    char *last_req;		/* beginning of last request, or dummy */
//    char *buffer;		/* Output buffer starting address. */
//    char *bufptr;		/* Output buffer index pointer. */
//    char *bufmax;		/* Output buffer maximum+1 address. */
//    unsigned max_request_size; /* maximum number 32 bit words in request*/
//    struct _XrmHashBucketRec *db;
//    int (*synchandler)();	/* Synchronization handler */
    char *display_name;	/* "host:display" string used on this connect*/
    int default_screen;	/* default screen for operations */
    int nscreens;		/* number of screens on this server*/
    Screen *screens;	/* pointer to list of screens */
//    unsigned long motion_buffer;	/* size of motion buffer */
//    unsigned long flags;	/* internal connection flags */
//    int min_keycode;	/* minimum defined keycode */
//    int max_keycode;	/* maximum defined keycode */
//    KeySym *keysyms;	/* This server's keysyms */
//    XModifierKeymap *modifiermap;	/* This server's modifier keymap */
//    int keysyms_per_keycode;/* number of rows */
//    char *xdefaults;	/* contents of defaults from server */
//    char *scratch_buffer;	/* place to hang scratch buffer */
//    unsigned long scratch_length;	/* length of scratch buffer */
//    int ext_number;		/* extension number on this display */
//    struct _XExten *ext_procs; /* extensions initialized on this display */
//    /*
//     * the following can be fixed size, as the protocol defines how
//     * much address space is available.
//     * While this could be done using the extension vector, there
//     * may be MANY events processed, so a search through the extension
//     * list to find the right procedure for each event might be
//     * expensive if many extensions are being used.
//     */
//    Bool (*event_vec[128])();  /* vector for wire to event */
//    Status (*wire_vec[128])(); /* vector for event to wire */
//    KeySym lock_meaning;	   /* for XLookupString */
//    struct _XLockInfo *lock;   /* multi-thread state, display lock */
//    struct _XInternalAsync *async_handlers; /* for internal async */
//    unsigned long bigreq_size; /* max size of big requests */
//    struct _XLockPtrs *lock_fns; /* pointers to threads functions */
//    /* things above this line should not move, for binary compatibility */
//    struct _XKeytrans *key_bindings; /* for XLookupString */
//    Font cursor_font;	   /* for XCreateFontCursor */
//    struct _XDisplayAtoms *atoms; /* for XInternAtom */
//    unsigned int mode_switch;  /* keyboard group modifiers */
//    struct _XContextDB *context_db; /* context database */
//    Bool (**error_vec)();      /* vector for wire to error */
//    /*
//     * Xcms information
//     */
//    struct {
//        XPointer defaultCCCs;  /* pointer to an array of default XcmsCCC */
//        XPointer clientCmaps;  /* pointer to linked list of XcmsCmapRec */
//        XPointer perVisualIntensityMaps;
//        /* linked list of XcmsIntensityMap */
//    } cms;
//    struct _XIMFilter *im_filters;
//    struct _XSQEvent *qfree; /* unallocated event queue elements */
    unsigned long next_event_serial_num; /* inserted into next queue elt */
//    int (*savedsynchandler)(); /* user synchandler when Xlib usurps */
} Display;

/*
 * per character font metric information.
 */
typedef struct {
//    short	lbearing;	/* origin to left edge of raster */
    short	rbearing;	/* origin to right edge of raster */
    short	width;		/* advance to next char's origin */
//    short	ascent;		/* baseline to top edge of raster */
//    short	descent;	/* baseline to bottom edge of raster */
//    unsigned short attributes;	/* per char flags (not predefined) */
} XCharStruct;

typedef struct {
//    XExtData	*ext_data;	/* hook for extension to hang data */
    Font        fid;            /* Font id for this font */
//    unsigned	direction;	/* hint about direction the font is painted */
    unsigned	min_char_or_byte2;/* first character */
    unsigned	max_char_or_byte2;/* last character */
    unsigned	min_byte1;	/* first row that exists */
    unsigned	max_byte1;	/* last row that exists */
//    Bool	all_chars_exist;/* flag if all characters have non-zero size*/
//    unsigned	default_char;	/* char to print for undefined character */
//    int         n_properties;   /* how many properties there are */
//    XFontProp	*properties;	/* pointer to array of additional properties*/
    XCharStruct	min_bounds;	/* minimum bounds over all existing char*/
    XCharStruct	max_bounds;	/* maximum bounds over all existing char*/
    XCharStruct	*per_char;	/* first_char to last_char information */
    int		ascent;		/* log. extent above baseline for spacing */
    int		descent;	/* log. descent below baseline for spacing */
} XFontStruct;

typedef struct {		/* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;

/* Data structure for X{Set,Get}ModifierMapping */

typedef struct {
    int max_keypermod;	/* The server's max # of keys per modifier */
    KeyCode *modifiermap;	/* An 8 by max_keypermod array of modifiers */
} XModifierKeymap;

// ---------------------------------------------------------------------------------------- //

/*
 * Definitions of specific events.
 */
typedef struct {
    int type;		/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;	        /* "event" window it is reported relative to */
    Window root;	        /* root window that the event occured on */
    Window subwindow;	/* child window */
    Time time;		/* milliseconds */
    int x, y;		/* pointer x, y coordinates in event window */
    int x_root, y_root;	/* coordinates relative to root */
    unsigned int state;	/* key or button mask */
    unsigned int keycode;	/* detail */
    Bool same_screen;	/* same screen flag */
    char trans_chars[XMaxTransChars];
    /* translated characters */
    int nbytes;
} XKeyEvent;

typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct {
    int type;		/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;	        /* "event" window it is reported relative to */
    Window root;	        /* root window that the event occured on */
    Window subwindow;	/* child window */
    Time time;		/* milliseconds */
    int x, y;		/* pointer x, y coordinates in event window */
    int x_root, y_root;	/* coordinates relative to root */
    unsigned int state;	/* key or button mask */
    unsigned int button;	/* detail */
    Bool same_screen;	/* same screen flag */
} XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
    int type;		/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;	        /* "event" window reported relative to */
    Window root;	        /* root window that the event occured on */
    Window subwindow;	/* child window */
    Time time;		/* milliseconds */
    int x, y;		/* pointer x, y coordinates in event window */
    int x_root, y_root;	/* coordinates relative to root */
    unsigned int state;	/* key or button mask */
    char is_hint;		/* detail */
    Bool same_screen;	/* same screen flag */
} XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {
    int type;		/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;	        /* "event" window reported relative to */
    Window root;	        /* root window that the event occured on */
    Window subwindow;	/* child window */
    Time time;		/* milliseconds */
    int x, y;		/* pointer x, y coordinates in event window */
    int x_root, y_root;	/* coordinates relative to root */
    int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
    int detail;
    /*
     * NotifyAncestor, NotifyVirtual, NotifyInferior,
     * NotifyNonlinear,NotifyNonlinearVirtual
     */
    Bool same_screen;	/* same screen flag */
    Bool focus;		/* boolean focus */
    unsigned int state;	/* key or button mask */
} XCrossingEvent;
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct {
    int type;		/* FocusIn or FocusOut */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;		/* window of event */
    int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
    int detail;
    /*
     * NotifyAncestor, NotifyVirtual, NotifyInferior,
     * NotifyNonlinear,NotifyNonlinearVirtual, NotifyPointer,
     * NotifyPointerRoot, NotifyDetailNone
     */
} XFocusChangeEvent;
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
typedef struct {
//    int type;
//    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Window window;
//    char key_vector[32];
} XKeymapEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;
    int x, y;
    int width, height;
    int count;		/* if non-zero, at least this many more */
} XExposeEvent;

typedef struct {
//    int type;
    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Drawable drawable;
    int x, y;
    int width, height;
    int count;		/* if non-zero, at least this many more */
//    int major_code;		/* core is CopyArea or CopyPlane */
//    int minor_code;		/* not defined in the core */
} XGraphicsExposeEvent;

typedef struct {
//    int type;
//    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Drawable drawable;
//    int major_code;		/* core is CopyArea or CopyPlane */
//    int minor_code;		/* not defined in the core */
} XNoExposeEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;
    int state;		/* Visibility state */
} XVisibilityEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window parent;		/* parent of the window */
    Window window;		/* window id of window created */
    int x, y;		/* window location */
    int width, height;	/* size of window */
    int border_width;	/* border width */
    Bool override_redirect;	/* creation should be overridden */
} XCreateWindowEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
} XDestroyWindowEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
    Bool from_configure;
} XUnmapEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
    Bool override_redirect;	/* boolean, is override set... */
} XMapEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window parent;
    Window window;
} XMapRequestEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
    Window parent;
    int x, y;
    Bool override_redirect;
} XReparentEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
    int x, y;
    int width, height;
    int border_width;
    Window above;
    Bool override_redirect;
} XConfigureEvent;

typedef struct {
//    int type;
//    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Window event;
//    Window window;
//    int x, y;
} XGravityEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;
    int width, height;
} XResizeRequestEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window parent;
    Window window;
    int x, y;
    int width, height;
    int border_width;
    Window above;
    int detail;		/* Above, Below, TopIf, BottomIf, Opposite */
    unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window event;
    Window window;
    int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window parent;
    Window window;
    int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;
    Atom atom;
    Time time;
    int state;		/* NewValue, Deleted */
} XPropertyEvent;

typedef struct {
//    int type;
    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Window window;
    Atom selection;
//    Time time;
} XSelectionClearEvent;

typedef struct {
//    int type;
    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
//    Window owner;
    Window requestor;
    Atom selection;
    Atom target;
    Atom property;
    Time time;
} XSelectionRequestEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window requestor;
    Atom selection;
    Atom target;
    Atom property;		/* ATOM or None */
    Time time;
} XSelectionEvent;

typedef struct {
//    int type;
//    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
//    Window window;
//    Colormap colormap;	/* COLORMAP or None */
//#if defined(__cplusplus) || defined(c_plusplus)
//    Bool c_new;		/* C++ */
//#else
//    Bool new;
//#endif
//    int state;		/* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

typedef struct {
//    int type;
    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
//    Display *display;	/* Display the event was read from */
    Window window;
    Atom message_type;
    int format;
    union {
        char b[20];
        short s[10];
        long l[5];
    } data;
} XClientMessageEvent;

typedef struct {
//    int type;
    unsigned long serial;	/* # of last request processed by server */
//    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;	/* Display the event was read from */
//    Window window;		/* unused */
//    int request;		/* one of MappingModifier, MappingKeyboard,
//				   MappingPointer */
//    int first_keycode;	/* first keycode */
//    int count;		/* defines range of change w. first_keycode*/
} XMappingEvent;

typedef struct {
    int type;
    Display *display;	/* Display the event was read from */
    XID resourceid;		/* resource id */
    unsigned long serial;	/* serial number of failed request */
    unsigned char error_code;	/* error code of failed request */
    unsigned char request_code;	/* Major op-code of failed request */
    unsigned char minor_code;	/* Minor op-code of failed request */
} XErrorEvent;

typedef struct {
    int type;
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came from a SendEvent request */
    Display *display;/* Display the event was read from */
    Window window;	/* window on which event was requested in event mask */
} XAnyEvent;

typedef union _XEvent {
    int type;		/* must not be changed; first element */
    XAnyEvent xany;
    XKeyEvent xkey;
    XButtonEvent xbutton;
    XMotionEvent xmotion;
    XCrossingEvent xcrossing;
    XFocusChangeEvent xfocus;
    XExposeEvent xexpose;
    XGraphicsExposeEvent xgraphicsexpose;
//    XNoExposeEvent xnoexpose;
    XVisibilityEvent xvisibility;
    XCreateWindowEvent xcreatewindow;
    XDestroyWindowEvent xdestroywindow;
    XUnmapEvent xunmap;
    XMapEvent xmap;
    XMapRequestEvent xmaprequest;
    XReparentEvent xreparent;
    XConfigureEvent xconfigure;
//    XGravityEvent xgravity;
    XResizeRequestEvent xresizerequest;
    XConfigureRequestEvent xconfigurerequest;
    XCirculateEvent xcirculate;
    XCirculateRequestEvent xcirculaterequest;
    XPropertyEvent xproperty;
    XSelectionClearEvent xselectionclear;
    XSelectionRequestEvent xselectionrequest;
    XSelectionEvent xselection;
//    XColormapEvent xcolormap;
    XClientMessageEvent xclient;
    XMappingEvent xmapping;
//    XErrorEvent xerror;
//    XKeymapEvent xkeymap;
    long pad[24];
} XEvent;

/* unused:
typedef void (*XOMProc)();
 */
typedef struct _XOM *XOM;
typedef struct _XOC *XOC, *XFontSet;
typedef struct {
    char           *chars;
    int             nchars;
    int             delta;
    XFontSet        font_set;
} XmbTextItem;
typedef struct {
    wchar_t        *chars;
    int             nchars;
    int             delta;
    XFontSet        font_set;
} XwcTextItem;
#define XNRequiredCharSet "requiredCharSet"
#define XNQueryOrientation "queryOrientation"
#define XNBaseFontName "baseFontName"
#define XNOMAutomatic "omAutomatic"
#define XNMissingCharSet "missingCharSet"
#define XNDefaultString "defaultString"
#define XNOrientation "orientation"
#define XNDirectionalDependentDrawing "directionalDependentDrawing"
#define XNContextualDrawing "contextualDrawing"
#define XNFontInfo "fontInfo"
typedef struct {
    int charset_count;
    char **charset_list;
} XOMCharSetList;
typedef enum {
    XOMOrientation_LTR_TTB,
    XOMOrientation_RTL_TTB,
    XOMOrientation_TTB_LTR,
    XOMOrientation_TTB_RTL,
    XOMOrientation_Context
} XOrientation;
typedef struct {
    int num_orientation;
    XOrientation *orientation;	/* Input Text description */
} XOMOrientation;
typedef struct {
    int num_font;
    XFontStruct **font_struct_list;
    char **font_name_list;
} XOMFontInfo;
typedef struct _XIM *XIM;
typedef struct _XIC *XIC;
typedef void (*XIMProc)(
    XIM,
    XPointer,
    XPointer
);
typedef Bool (*XICProc)(
    XIC,
    XPointer,
    XPointer
);
typedef void (*XIDProc)(
    Display*,
    XPointer,
    XPointer
);
typedef unsigned long XIMStyle;
typedef struct {
    unsigned short count_styles;
    XIMStyle *supported_styles;
} XIMStyles;
#define XIMPreeditArea		0x0001L
#define XIMPreeditCallbacks	0x0002L
#define XIMPreeditPosition	0x0004L
#define XIMPreeditNothing	0x0008L
#define XIMPreeditNone		0x0010L
#define XIMStatusArea		0x0100L
#define XIMStatusCallbacks	0x0200L
#define XIMStatusNothing	0x0400L
#define XIMStatusNone		0x0800L
#define XNVaNestedList "XNVaNestedList"
#define XNQueryInputStyle "queryInputStyle"
#define XNClientWindow "clientWindow"
#define XNInputStyle "inputStyle"
#define XNFocusWindow "focusWindow"
#define XNResourceName "resourceName"
#define XNResourceClass "resourceClass"
#define XNGeometryCallback "geometryCallback"
#define XNDestroyCallback "destroyCallback"
#define XNFilterEvents "filterEvents"
#define XNPreeditStartCallback "preeditStartCallback"
#define XNPreeditDoneCallback "preeditDoneCallback"
#define XNPreeditDrawCallback "preeditDrawCallback"
#define XNPreeditCaretCallback "preeditCaretCallback"
#define XNPreeditStateNotifyCallback "preeditStateNotifyCallback"
#define XNPreeditAttributes "preeditAttributes"
#define XNStatusStartCallback "statusStartCallback"
#define XNStatusDoneCallback "statusDoneCallback"
#define XNStatusDrawCallback "statusDrawCallback"
#define XNStatusAttributes "statusAttributes"
#define XNArea "area"
#define XNAreaNeeded "areaNeeded"
#define XNSpotLocation "spotLocation"
#define XNColormap "colorMap"
#define XNStdColormap "stdColorMap"
#define XNForeground "foreground"
#define XNBackground "background"
#define XNBackgroundPixmap "backgroundPixmap"
#define XNFontSet "fontSet"
#define XNLineSpace "lineSpace"
#define XNCursor "cursor"
#define XNQueryIMValuesList "queryIMValuesList"
#define XNQueryICValuesList "queryICValuesList"
#define XNVisiblePosition "visiblePosition"
#define XNR6PreeditCallback "r6PreeditCallback"
#define XNStringConversionCallback "stringConversionCallback"
#define XNStringConversion "stringConversion"
#define XNResetState "resetState"
#define XNHotKey "hotKey"
#define XNHotKeyState "hotKeyState"
#define XNPreeditState "preeditState"
#define XNSeparatorofNestedList "separatorofNestedList"
#define XBufferOverflow		-1
#define XLookupNone		1
#define XLookupChars		2
#define XLookupKeySym		3
#define XLookupBoth		4
typedef void *XVaNestedList;
typedef struct {
    XPointer client_data;
    XIMProc callback;
} XIMCallback;
typedef struct {
    XPointer client_data;
    XICProc callback;
} XICCallback;
typedef unsigned long XIMFeedback;
#define XIMReverse		1L
#define XIMUnderline		(1L<<1) 
#define XIMHighlight		(1L<<2)
#define XIMPrimary	 	(1L<<5)
#define XIMSecondary		(1L<<6)
#define XIMTertiary	 	(1L<<7)
#define XIMVisibleToForward 	(1L<<8)
#define XIMVisibleToBackword 	(1L<<9)
#define XIMVisibleToCenter 	(1L<<10)
typedef struct _XIMText {
    unsigned short length;
    XIMFeedback *feedback;
    Bool encoding_is_wchar; 
    union {
	char *multi_byte;
	wchar_t *wide_char;
    } string; 
} XIMText;
typedef	unsigned long	 XIMPreeditState;
#define	XIMPreeditUnKnown	0L
#define	XIMPreeditEnable	1L
#define	XIMPreeditDisable	(1L<<1)
typedef	struct	_XIMPreeditStateNotifyCallbackStruct {
    XIMPreeditState state;
} XIMPreeditStateNotifyCallbackStruct;
typedef	unsigned long	 XIMResetState;
#define	XIMInitialState		1L
#define	XIMPreserveState	(1L<<1)
typedef unsigned long XIMStringConversionFeedback;
#define	XIMStringConversionLeftEdge	(0x00000001)
#define	XIMStringConversionRightEdge	(0x00000002)
#define	XIMStringConversionTopEdge	(0x00000004)
#define	XIMStringConversionBottomEdge	(0x00000008)
#define	XIMStringConversionConcealed	(0x00000010)
#define	XIMStringConversionWrapped	(0x00000020)
typedef struct _XIMStringConversionText {
    unsigned short length;
    XIMStringConversionFeedback *feedback;
    Bool encoding_is_wchar; 
    union {
	char *mbs;
	wchar_t *wcs;
    } string; 
} XIMStringConversionText;
typedef	unsigned short	XIMStringConversionPosition;
typedef	unsigned short	XIMStringConversionType;
#define	XIMStringConversionBuffer	(0x0001)
#define	XIMStringConversionLine		(0x0002)
#define	XIMStringConversionWord		(0x0003)
#define	XIMStringConversionChar		(0x0004)
typedef	unsigned short	XIMStringConversionOperation;
#define	XIMStringConversionSubstitution	(0x0001)
#define	XIMStringConversionRetrieval	(0x0002)
typedef enum {
    XIMForwardChar, XIMBackwardChar,
    XIMForwardWord, XIMBackwardWord,
    XIMCaretUp, XIMCaretDown,
    XIMNextLine, XIMPreviousLine,
    XIMLineStart, XIMLineEnd, 
    XIMAbsolutePosition,
    XIMDontChange
} XIMCaretDirection;
typedef struct _XIMStringConversionCallbackStruct {
    XIMStringConversionPosition position;
    XIMCaretDirection direction;
    XIMStringConversionOperation operation;
    unsigned short factor;
    XIMStringConversionText *text;
} XIMStringConversionCallbackStruct;
typedef struct _XIMPreeditDrawCallbackStruct {
    int caret;		/* Cursor offset within pre-edit string */
    int chg_first;	/* Starting change position */
    int chg_length;	/* Length of the change in character count */
    XIMText *text;
} XIMPreeditDrawCallbackStruct;
typedef enum {
    XIMIsInvisible,	/* Disable caret feedback */ 
    XIMIsPrimary,	/* UI defined caret feedback */
    XIMIsSecondary	/* UI defined caret feedback */
} XIMCaretStyle;
typedef struct _XIMPreeditCaretCallbackStruct {
    int position;		 /* Caret offset within pre-edit string */
    XIMCaretDirection direction; /* Caret moves direction */
    XIMCaretStyle style;	 /* Feedback of the caret */
} XIMPreeditCaretCallbackStruct;
typedef enum {
    XIMTextType,
    XIMBitmapType
} XIMStatusDataType;
	
typedef struct _XIMStatusDrawCallbackStruct {
    XIMStatusDataType type;
    union {
	XIMText *text;
	Pixmap  bitmap;
    } data;
} XIMStatusDrawCallbackStruct;
typedef struct _XIMHotKeyTrigger {
    KeySym	 keysym;
    int		 modifier;
    int		 modifier_mask;
} XIMHotKeyTrigger;
typedef struct _XIMHotKeyTriggers {
    int			 num_hot_key;
    XIMHotKeyTrigger	*key;
} XIMHotKeyTriggers;
typedef	unsigned long	 XIMHotKeyState;
#define	XIMHotKeyStateON	(0x0001L)
#define	XIMHotKeyStateOFF	(0x0002L)
typedef struct {
    unsigned short count_values;
    char **supported_values;
} XIMValuesList;

/* API mentioning "UTF8" or "utf8" is an XFree86 extension, introduced in
   November 2000. Its presence is indicated through the following macro. */
#define X_HAVE_UTF8_STRING 1

#endif /* XLIB_H */