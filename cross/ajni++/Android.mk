LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := ajni++
LOCAL_MODULE_FILENAME := libajni++
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../

LOCAL_SRC_FILES := \
ajni++.cpp \
android.cpp \
core.cpp \
inspect.cpp \
jre.cpp \
variant.cpp

include $(BUILD_STATIC_LIBRARY)
