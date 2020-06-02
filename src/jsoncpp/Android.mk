LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := jsoncpp
LOCAL_MODULE_FILENAME := libjsoncpp
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
json_reader.cpp \
json_value.cpp \
json_writer.cpp

LOCAL_CPPFLAGS += -std=c++11 -fexceptions
LOCAL_CPPFLAGS += -DJSONCPP_NO_LOCALE_SUPPORT

include $(BUILD_STATIC_LIBRARY)
