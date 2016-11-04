#include "X11/Xlib.h"
#include "errors.h"
#include "drawing.h"
#include "resourceTypes.h"
#include "window.h"
#include "display.h"

// Inspired by https://github.com/csulmone/X11/blob/59029dc09211926a5c95ff1dd2b828574fefcde6/libX11-1.5.0/src/ImUtil.c

XImage* XCreateImage(Display* display, Visual* visual, unsigned int depth, int format, int offset,
                     char* data, unsigned int width, unsigned int height, int bitmap_pad,
                     int bytes_per_line) {
    // https://tronche.com/gui/x/xlib/utilities/XCreateImage.html
    SET_X_SERVER_REQUEST(display, XCB_CREATE_IMAGE);
    XImage* image = malloc(sizeof(XImage));
    if (image == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    fprintf(stderr, "%s: w = %d, h = %d\n", __func__, (int) width, (int) height);
    image->width = width;
    image->height = height;
    image->format = format;
    image->data = data;
    image->byte_order = MSBFirst;
    image->bitmap_unit = 8;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    image->bitmap_bit_order = MSBFirst;
    #else
    image->bitmap_bit_order = LSBFirst;
    #endif
    image->depth = depth;
    image->bytes_per_line = bytes_per_line;
    if (format != ZPixmap) {
        image->bits_per_pixel = 1;
    } else {
        if (depth <= 4) {
            image->bits_per_pixel = 4;
        } else if (depth <= 8) {
            image->bits_per_pixel = 8;
        } else if (depth <= 16) {
            image->bits_per_pixel = 16;
        } else {
            image->bits_per_pixel = 32;
        }
    }
    return image;
}

Status XInitImage(XImage* image) {
    // https://tronche.com/gui/x/xlib/graphics/XInitImage.html
    return 1;
}

Status _XInitImageFuncPtrs(XImage *image) {
    return XInitImage(image);
}

char* getImageDataPointer(XImage* image, int x, int y) {
    char* pointer = image->data;
    pointer += image->bytes_per_line * y;
    return  pointer + (image->bits_per_pixel / 8) * y;
}

void XPutPixel(XImage* image, int x, int y, unsigned long pixel) {
    // https://tronche.com/gui/x/xlib/utilities/XPutPixel.html
    fprintf(stderr, "%s on °%p: %lu (%ld, %ld, %ld)\n", __func__, image, pixel, (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF);
    if (image->data == NULL) {
        fprintf(stderr, "Invalid argument: Got image with NULL data in XPutPixel\n");
        return;
    }
    char* pointer;
    switch (image->format) {
        case ZPixmap:
        case XYBitmap:
            pointer = getImageDataPointer(image, x, y);
            if (image->bits_per_pixel == 32) {
                *((unsigned long*) pointer) = pixel;
            }
            break;
        case XYPixmap:
            fprintf(stderr, "Warn: Got unimplemented format XYPixmap\n");
            break;
        default:
            fprintf(stderr, "Warn: Got invalid format %d\n", image->format);
    }
}

unsigned long XGetPixel(XImage* image, int x, int y) {
    // https://tronche.com/gui/x/xlib/utilities/XGetPixel.html
    fprintf(stderr, "%s from %p: x = %d, y = %d\n", __func__, image, x, y);
    if (image->data == NULL) {
        fprintf(stderr, "Invalid argument: Got image with NULL data in XGetPixel\n");
        return 0; // TODO: throw error
    }
    char* pointer;
    switch (image->format) {
        case ZPixmap:
        case XYBitmap:
            pointer = getImageDataPointer(image, x, y);
//            fprintf(stderr, "%s: bits_per_pixel = %d, value = %x (%d)\n", __func__, image->bits_per_pixel, *pointer & 0xFF, (int) *pointer & 0xFF);
            if (image->bits_per_pixel == 32) {
                return *((unsigned long*) pointer);
            } else if (image->bits_per_pixel == 1) {
                return 0x000000FF | (*pointer << 12) | (*pointer << 8) | (*pointer << 4);
            }
            break;
        case XYPixmap:
            fprintf(stderr, "Warn: Got unimplemented format XYPixmap\n");
            break;
        default:
            fprintf(stderr, "Warn: Got invalid format %d\n", image->format);
    }
    return 0;
}

void XDestroyImage(XImage* image) {
    // https://tronche.com/gui/x/xlib/utilities/XDestroyImage.html
    if (image->data != NULL) {
        free((char*) image->data);
    }
    free(image);
}

void XPutImage(Display* display, Drawable drawable, GC gc, XImage* image, int src_x, int src_y,
               int dest_x, int dest_y, unsigned int width, unsigned int height) {
    // https://tronche.com/gui/x/xlib/graphics/XPutImage.html
    SET_X_SERVER_REQUEST(display, XCB_PUT_IMAGE);
    TYPE_CHECK(drawable, DRAWABLE, display);
    fprintf(stderr, "%s: Drawing %p on %p\n", __func__, image, drawable);
    // TODO: Implement this: Create Uint32* data, Create Texture from data, rendercopy
//    LOCK_SURFACE(surface);
//    unsigned int x, y;
//    if (image->format == XYBitmap && 0) {
//        for (x = 0; x < width; x++) {
//            for (y = 0; y < height; y++) {
//                unsigned long color = gc->background;
//                if (XGetPixel(image, src_x + x, src_y + y)) {
//                    color = gc->foreground;
//                }
//                putPixel(surface, dest_x + x, dest_y + y, color);
//            }
//        }
//    } else {
//        for (x = 0; x < width; x++) {
//            for (y = 0; y < height; y++) {
//                unsigned long color = XGetPixel(image, src_x + x, src_y + y);
////                fprintf(stderr, "%s: %lu (%ld, %ld, %ld)\n", __func__, color, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF);
//                putPixel(surface, dest_x + x, dest_y + y, XGetPixel(image, src_x + x, src_y + y));
//            }
//        }
//    }
//    UNLOCK_SURFACE(surface);
}

XImage* XGetImage(Display* display, Drawable drawable, int x, int y, unsigned int width,
                  unsigned int height, unsigned long plane_mask, int format) {
    // https://tronche.com/gui/x/xlib/graphics/XGetImage.html
    SET_X_SERVER_REQUEST(display, XCB_GET_IMAGE);
    fprintf(stderr, "%s: From %p\n", __func__, drawable);
    if (IS_TYPE(drawable, WINDOW) && drawable->dataPointer == SCREEN_WINDOW) {
        fprintf(stderr, "XGetImage called with SCREEN_WINDOW as argument!\n");
        // TODO: Handle error and check INPUTONLY
        return NULL;
    }
    // TODO: Figure this out
    int depth = 8;
//    if (drawable->type == PIXMAP) {
//
//    }
    int bytes_per_line = (depth / 8) * width;
    char* data = malloc(sizeof(char) * width * height);
    if (data == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    // FIXME: The NULL will cause problems.
    XImage* image = XCreateImage(display, NULL, depth, format, 0, data, width, height,
                                 (int) NULL, bytes_per_line);
    if (image == NULL) {
        free(data);
        return NULL;
    }
    //TODO: Implement: Read from Textur into data and Convert from data to image type
//    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, GET_SURFACE(drawable)->pixels, GET_SURFACE(drawable)->pitch);

//    unsigned int currX, currY;
//    // TODO: Worry about XYPixmap
////    if (format == XYPixmap) {
//    for (currX = 0; currX < width; currX++) {
//        for (currY = 0; currY < height; currY++) {
//            data[currY * width + currX] = plane_mask & getPixel(drawableSurface, x + currX, y + currY);
//        }
//    }
    return image;
}
