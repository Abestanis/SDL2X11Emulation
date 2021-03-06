LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := sdl2X11Emulation
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/src/*.c)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

ifeq ($(COMPILE_DEV_LIBS), true)
  LOCAL_CFLAGS += -D DEBUG_SDL2X11_EMULATION
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_gpu SDL2_ttf pixman

include $(BUILD_SHARED_LIBRARY)
