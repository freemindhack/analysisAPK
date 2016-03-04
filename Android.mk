LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
FILE_LIST := $(wildcard $(LOCAL_PATH)/src/*.c)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%) \
	src/tinyxml.cpp \
	src/tinyxmlparser.cpp \
	src/tinyxmlerror.cpp \
	src/tinystr.cpp \
	src/AxmlParser.cpp\
	src/FileUtil.cpp\
	src/ParseUtil.cpp\
	src/analysisAPK.cpp 
LOCAL_MODULE := ApkAnalysis
LOCAL_LDLIBS := -llog
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
