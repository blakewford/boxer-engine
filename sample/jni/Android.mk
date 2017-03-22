LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := boxerEngine
LOCAL_SRC_FILES := ../../lib/libboxer/obj/local/$(TARGET_ARCH_ABI)/libboxer.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := boxer
LOCAL_CFLAGS += -std=c++11 -I../include
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := boxerEngine
LOCAL_SRC_FILES := ../main.cpp

include $(BUILD_SHARED_LIBRARY)
