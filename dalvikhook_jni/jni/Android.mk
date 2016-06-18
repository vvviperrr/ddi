LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := dalvikhook_jni 

LOCAL_SRC_FILES := \
	unboxing.c \
	hook_entry.c \
	dalvikhook_jni.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../../../adbi/instruments/base/ \
	$(LOCAL_PATH)/../../../../dalvikhook/jni/ \
	$(LOCAL_PATH)/../../../inc

LOCAL_LDLIBS := \
	-L../../../dalvikhook/jni/libs \
	-ldl -ldvm -llog \
	-Wl,--start-group ../../../../adbi/instruments/base/obj/local/armeabi/libbase.a \
	../../../dalvikhook/obj/local/armeabi/libdalvikhook.a -Wl,--end-group

LOCAL_CFLAGS := -g #-Wall -DPRODUCTION_LOG

include $(BUILD_SHARED_LIBRARY)
