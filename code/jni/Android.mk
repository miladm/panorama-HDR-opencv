LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Tegra optimized OpenCV.mk
include /Users/milmil/Documents/NVPACK/OpenCV-2.4.5-Tegra-sdk-r2/sdk/native/jni/OpenCV-tegra3.mk

# Linker
LOCAL_LDLIBS += -llog

# Our module sources
LOCAL_MODULE    := PanoHDR
LOCAL_SRC_FILES := PanoHDR.cpp Panorama.cpp HDR.cpp NativeLogging.cpp

include $(BUILD_SHARED_LIBRARY)
