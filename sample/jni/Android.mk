LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := boxer
LOCAL_CFLAGS += -std=c++11 -I../include
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := ../main.cpp ../../lib/libboxer/boxer.cpp ../../lib/libboxer/stage.cpp

include $(BUILD_SHARED_LIBRARY)
