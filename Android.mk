LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= dht11test.c
LOCAL_MODULE := dht11test
LOCAL_MODULE_TAGS :=eng
include $(BUILD_EXECUTABLE)
