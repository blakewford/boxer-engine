LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := boxer
LOCAL_CFLAGS += -std=c++11 -I../../include
LOCAL_SRC_FILES := ../boxer.cpp ../stage.cpp ../ndk.cpp ../android.cpp

include $(BUILD_STATIC_LIBRARY)
