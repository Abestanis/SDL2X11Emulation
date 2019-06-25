cmake_minimum_required(VERSION 3.4.1)
project(SDL2X11Emulation C)

set(CMAKE_C_FLAGS_DEBUG -DDEBUG_SDL2X11_EMULATION)

add_library(sdl2X11Emulation SHARED
        include/X11/DECkeysym.h include/X11/HPkeysym.h include/X11/ImUtil.h
        include/X11/Sunkeysym.h include/X11/X.h include/X11/XF86keysym.h
        include/X11/XKBlib.h include/X11/XWDFile.h include/X11/Xalloca.h
        include/X11/Xarch.h include/X11/Xatom.h include/X11/Xauth.h
        include/X11/Xcms.h include/X11/Xdefs.h include/X11/Xdmcp.h
        include/X11/Xfuncproto.h include/X11/Xfuncs.h include/X11/Xlib.h
        include/X11/XlibConf.h include/X11/Xlibint.h include/X11/Xlocale.h
        include/X11/Xmd.h include/X11/Xos.h include/X11/Xos_r.h
        include/X11/Xosdefs.h include/X11/Xpoll.h include/X11/Xproto.h
        include/X11/Xprotostr.h include/X11/Xregion.h include/X11/Xresource.h
        include/X11/Xthreads.h include/X11/Xtrans/Xtrans.h
        include/X11/Xtrans/Xtransint.h include/X11/Xutil.h include/X11/Xw32defs.h
        include/X11/Xwindows.h include/X11/Xwinsock.h include/X11/ap_keysym.h
        include/X11/bitmaps/1x1 include/X11/bitmaps/2x2 include/X11/bitmaps/Dashes
        include/X11/bitmaps/Down include/X11/bitmaps/Excl include/X11/bitmaps/FlipHoriz
        include/X11/bitmaps/FlipVert include/X11/bitmaps/Fold include/X11/bitmaps/Left
        include/X11/bitmaps/Right include/X11/bitmaps/RotateLeft include/X11/bitmaps/RotateRight
        include/X11/bitmaps/Stipple include/X11/bitmaps/Term include/X11/bitmaps/Up
        include/X11/bitmaps/black include/X11/bitmaps/black6 include/X11/bitmaps/box6
        include/X11/bitmaps/boxes include/X11/bitmaps/calculator include/X11/bitmaps/cntr_ptr
        include/X11/bitmaps/cntr_ptrmsk include/X11/bitmaps/cross_weave include/X11/bitmaps/dimple1
        include/X11/bitmaps/dimple3 include/X11/bitmaps/dot include/X11/bitmaps/dropbar7
        include/X11/bitmaps/dropbar8 include/X11/bitmaps/escherknot include/X11/bitmaps/flagdown
        include/X11/bitmaps/flagup include/X11/bitmaps/flipped_gray include/X11/bitmaps/gray
        include/X11/bitmaps/gray1 include/X11/bitmaps/gray3 include/X11/bitmaps/grid16
        include/X11/bitmaps/grid2 include/X11/bitmaps/grid4 include/X11/bitmaps/grid8
        include/X11/bitmaps/hlines2 include/X11/bitmaps/hlines3 include/X11/bitmaps/icon
        include/X11/bitmaps/keyboard16 include/X11/bitmaps/ldblarrow include/X11/bitmaps/left_ptr
        include/X11/bitmaps/left_ptrmsk include/X11/bitmaps/letters include/X11/bitmaps/light_gray
        include/X11/bitmaps/mailempty include/X11/bitmaps/mailemptymsk include/X11/bitmaps/mailfull
        include/X11/bitmaps/mailfullmsk include/X11/bitmaps/mensetmanus include/X11/bitmaps/menu10
        include/X11/bitmaps/menu12 include/X11/bitmaps/menu16 include/X11/bitmaps/menu6
        include/X11/bitmaps/menu8 include/X11/bitmaps/noletters include/X11/bitmaps/opendot
        include/X11/bitmaps/opendotMask include/X11/bitmaps/plaid include/X11/bitmaps/rdblarrow
        include/X11/bitmaps/right_ptr include/X11/bitmaps/right_ptrmsk
        include/X11/bitmaps/root_weave include/X11/bitmaps/scales include/X11/bitmaps/sipb
        include/X11/bitmaps/star include/X11/bitmaps/starMask include/X11/bitmaps/target
        include/X11/bitmaps/terminal include/X11/bitmaps/tie_fighter include/X11/bitmaps/vlines2
        include/X11/bitmaps/vlines3 include/X11/bitmaps/weird_size include/X11/bitmaps/wide_weave
        include/X11/bitmaps/wingdogs include/X11/bitmaps/woman include/X11/bitmaps/xfd_icon
        include/X11/bitmaps/xlogo11 include/X11/bitmaps/xlogo16 include/X11/bitmaps/xlogo32
        include/X11/bitmaps/xlogo64 include/X11/bitmaps/xsnow include/X11/cursorfont.h
        include/X11/extensions/XI.h include/X11/extensions/XI2.h include/X11/extensions/XI2proto.h
        include/X11/extensions/XIproto.h include/X11/extensions/XKB.h
        include/X11/extensions/XKBgeom.h include/X11/extensions/XKBproto.h
        include/X11/extensions/XKBsrv.h include/X11/extensions/XKBstr.h
        include/X11/keysym.h include/X11/keysymdef.h include/xbytes.h
        src/atomList.h src/atoms.c src/atoms.h src/colors.c src/colors.h
        src/cursor.c src/display.c src/display.h src/drawing.c src/drawing.h
        src/error.c src/errors.h src/events.c src/events.h src/font.c src/font.h
        src/gc.c src/gc.h src/image.c src/input.c src/input.h
        src/inputMethod.c src/inputMethod.h src/keysymlist.h src/netAtoms.h
        src/pixmap.c src/pointer.c src/region.c src/resourceTypes.h
        src/screensaver.c src/stdColors.h src/util.c src/util.h
        src/visual.c src/visual.h src/window.c src/window.h src/windowDebug.c
        src/windowDebug.h src/windowInternal.c src/windowInternal.h)

target_include_directories(sdl2X11Emulation
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_libraries(
        sdl2X11Emulation
        SDL2 SDL_gpu_shared SDL2_ttf pixman)
