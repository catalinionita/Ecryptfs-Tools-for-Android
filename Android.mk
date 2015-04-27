#
# Copyright (C) 2013 Intel Corporation, All Rights Reserved
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Catalin Ionita <catalin.ionita@intel.com>
#


LOCAL_PATH:= $(call my-dir)

ifneq ($(APPLY_EFS_PATCHES),)
APPLY_EFS_PATCHES:= $(shell $(LOCAL_PATH)/android_integration/apply_patches.sh)
endif

# libefs shared library
include $(CLEAR_VARS)
LOCAL_MODULE := libefs
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:= \
	src/lib/efs/efs.c \
	src/lib/efs/key_chain.c \
	src/lib/efs/file_utils.c \
	src/lib/efs/mount_utils.c \
	src/lib/efs/crypto.c \
	src/lib/efs/key_store.c
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src/include \
	$(LOCAL_PATH)/linux_headers/include \
	external/openssl/include
LOCAL_SHARED_LIBRARIES := \
	libcrypto \
	libcutils \
	liblog \
	libselinux
LOCAL_CFLAGS += -Wall
include $(BUILD_SHARED_LIBRARY)

# libefs_init - small static library to be built with init
include $(CLEAR_VARS)
LOCAL_MODULE := libefs_init
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES += src/lib/init/init.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CFLAGS += -Wall
include $(BUILD_STATIC_LIBRARY)

# custom demo lib for full user data encryption
include $(CLEAR_VARS)
LOCAL_MODULE := libUserEncryption
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES:= \
	src/lib/user_data_encryption/android_user_encryption.c \
	src/lib/user_data_encryption/process.c
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src/include \
	$(LOCAL_PATH)/linux_headers/include \
	external/openssl/include
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libhardware_legacy \
	libefs
LOCAL_CFLAGS += -Wall
include $(BUILD_SHARED_LIBRARY)

# library testing tool
include $(CLEAR_VARS)
LOCAL_MODULE:= efs-tools
LOCAL_MODULE_TAGS := eng
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES := src/tools/efs-tools.c
LOCAL_SHARED_LIBRARIES := libefs
include $(BUILD_EXECUTABLE)

# native service testing tool
include $(CLEAR_VARS)
LOCAL_MODULE:= edc
LOCAL_MODULE_TAGS := eng
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES := src/tools/edc.c
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)

# efs native service
include $(CLEAR_VARS)
LOCAL_MODULE := efs-server
LOCAL_MODULE_TAGS := eng
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CPPFLAGS += -Wall
LOCAL_SRC_FILES := \
    src/service/EfsServer.cpp \
    src/service/CommandListener.cpp
LOCAL_SHARED_LIBRARIES := \
    libefs \
    libcutils \
    liblog \
    libsysutils \
    libUserEncryption
include $(BUILD_EXECUTABLE)

include $(wildcard $(LOCAL_PATH)/*/*/Android.mk)

