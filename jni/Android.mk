# https://developer.android.com/ndk/guides/android_mk.html

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := raylib
LOCAL_SRC_FILES := raylib/src/rcore.c raylib/src/rshapes.c raylib/src/rtextures.c raylib/src/rtext.c raylib/src/rmodels.c raylib/src/utils.c raylib/src/raudio.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/raylib/src
LOCAL_CFLAGS := -DPLATFORM_ANDROID -DGRAPHICS_API_OPENGL_ES2
LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/raylib/src/ 

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_SRC_FILES := main.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/raylib/src
#LOCAL_CFLAGS
#LOCAL_CPPFLAGS
#LOCAL_LDFLAGS
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue raylib
#LOCAL_SHARED_LIBRARIES
#LOCAL_ARM_MODE := arm
#LOCAL_ARM_NEON
#LOCAL_EXPORT_CFLAGS
#LOCAL_EXPORT_CPPFLAGS
#LOCAL_EXPORT_C_INCLUDES
#LOCAL_EXPORT_LDFLAGS
#LOCAL_EXPORT_LDLIBS

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
