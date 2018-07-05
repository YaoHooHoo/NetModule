LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libAacFdk
LOCAL_SRC_FILES	:= lib_a/libAacFdk.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libAacSys
LOCAL_SRC_FILES	:= lib_a/libAacSys.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libAacEncoder
LOCAL_SRC_FILES	:= lib_a/libAacEncoder.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libAacDecoder
LOCAL_SRC_FILES	:= lib_a/libAacDecoder.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libSBRenc
LOCAL_SRC_FILES	:= lib_a/libSBRenc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libSBRdec
LOCAL_SRC_FILES	:= lib_a/libSBRdec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libMpegTPEnc
LOCAL_SRC_FILES	:= lib_a/libMpegTPEnc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libMpegTPDec
LOCAL_SRC_FILES	:= lib_a/libMpegTPDec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libPCMutils
LOCAL_SRC_FILES	:= lib_a/libPCMutils.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libx264Encode
LOCAL_SRC_FILES	:= lib_a/libx264Encode.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= libx264
LOCAL_SRC_FILES	:= lib_a/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS += -fexceptions

LOCAL_MODULE    := OitAVManage
LOCAL_SRC_FILES := AVManageJni.cpp utils.cpp \
	../common/ServiceThread.cpp ../common/globaldef.cpp	../common/myFile.cpp ../common/ProtocolFunction.cpp	\
	MediaManager.cpp VideoEncodeThread.cpp	ScreenCapThread.cpp	EncOrDec.cpp \
	ClientSocket.cpp NetDataSyncSink.cpp  FlvToFileLinux.cpp TimerEngine.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) /..	\
					$(LOCAL_PATH)/../common \
					$(LOCAL_PATH)/../AudioModule/libAudioDecoder \
					$(LOCAL_PATH)/../AudioModule/libAudioEncoder \
					$(LOCAL_PATH)/../AudioModule/audio_device/include \
					$(LOCAL_PATH)/../AudioModule/audio_device/android	\
					$(LOCAL_PATH)/../AudioModule	\
					$(LOCAL_PATH)/../AudioModule/interface \
					$(LOCAL_PATH)/../AudioModule/interface2 \
					$(LOCAL_PATH)/../AudioModule/include	\
					$(LOCAL_PATH)/../aacInclude	\
					$(LOCAL_PATH)/../x264Include	\
					$(JNI_H_INCLUDE)


ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS := -DHAVE_NEON=1
    LOCAL_SRC_FILES += helloneon-intrinsics.c.neon
endif

LOCAL_LDFLAGS += -fPIC

# for native audio
LOCAL_LDLIBS    += -lOpenSLES
# for logging
LOCAL_LDLIBS    += -llog
# for native asset manager
LOCAL_LDLIBS    += -landroid

LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS

APP_CFLAGS += -Wno-error=format-security

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libdl \
    libstlport \
    libOpenSLES 
    
    
LOCAL_STATIC_LIBRARIES := libAudioEngine \
                          libwebrtc_audio_device \
						  libapm \
                          libwebrtc_vad \
                          libwebrtc_agc \
                          libwebrtc_aec \
                          libwebrtc_ns \
                          libwebrtc_aecm \
                          libwebrtc_system_wrappers \
                          libwebrtc_spl \
                          libwebrtc_apm_utility \
                          libthread_util \
                          libtimer_util	\
						  libx264Encode	\
						  libx264	\
						  libAacEncoder libAacDecoder libSBRenc libMpegTPEnc libAacFdk libAacSys libPCMutils libSBRdec libMpegTPDec

include $(BUILD_SHARED_LIBRARY)
