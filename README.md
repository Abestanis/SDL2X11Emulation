# SDL2X11Emulation
This library was designed to allow programs that depend on [X11](https://en.wikipedia.org/wiki/X_Window_System)
to be run on platforms that don't have an XServer.
It implements all the Xlib API functions with [SDL2](https://www.libsdl.org/) and [SDL_gpu](https://grimfang4.github.io/sdl-gpu/).

Note: SDL_gpu requires OpenGl, OpenGL ES or some graphics API to be avaliable on the target system. If your target system does not support that, you could try to revert the changes made in [this patch](https://github.com/Abestanis/SDL2X11Emulation/commit/cb26a90e22837256a31dbd89abf477377000f9bb) and see if you can get away with just SDL.

> [!NOTE]
> This library is currently not actively maintained, but [`libx11-compat`](https://github.com/sysprog21/libx11-compat) is an actively
> maintained fork which is more feature complete and tested.

## License
Copied from [X.h](https://github.com/Abestanis/SDL2X11Emulation/blob/master/include/X11/X.h#L11):
```
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
```

## Background
I designed this library in order to allow [Tk](https://www.tcl.tk/) to run on Android.
This was a goal I wanted to achieve for my [APython project](https://github.com/Abestanis/APython).

## Progress and Contribution
This library is not finished and it will probably never have the performance of the real X-System.
I never heard of X11 before I started this project and the documentation is not really easy to understand.
Also, I never used SDL before.
I consider [`libx11-compat`](https://github.com/sysprog21/libx11-compat) to be the successor of this project,
so for everyone interested in contributing or using it I would suggest to check them out.

Thank you!
