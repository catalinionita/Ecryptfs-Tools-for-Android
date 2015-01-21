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
	src/efs.c \
	src/android_user_encryption.c \
	src/key_chain.c \
	src/file_utils.c \
	src/mount_utils.c \
	src/process.c \
	src/crypto.c \
	src/key_store.c
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src/include \
	$(LOCAL_PATH)/linux_headers/include \
	external/openssl/include
LOCAL_SHARED_LIBRARIES := \
	libcrypto \
	libcutils \
	liblog \
	libhardware_legacy \
	libselinux
LOCAL_CFLAGS += -Wall
include $(BUILD_SHARED_LIBRARY)

# libefs_init - small static library to be built with init
include $(CLEAR_VARS)
LOCAL_MODULE := libefs_init
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES += src/init.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CFLAGS += -Wall
include $(BUILD_STATIC_LIBRARY)

# libefs testing tool
include $(CLEAR_VARS)
LOCAL_MODULE:= efs-tools
LOCAL_MODULE_TAGS := eng
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/include
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES := src/main.c
LOCAL_SHARED_LIBRARIES := libefs
include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/test/Android.mk

