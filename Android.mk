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
APPLY_EFS_PATCHES:= $(shell vendor/intel/efs-tools/android_integration/apply_patches.sh)
endif

lib_keyutil_src:= \
	lib/keyutil/keyutil.c

lib_efs_src_files:= \
	src/efs.c \
	src/android_user_encryption.c \
	src/key_chain.c \
	src/file_utils.c \
	src/mount_utils.c \
	src/process.c \
	src/crypto.c \
	src/key_store.c

lib_efs_init:= init/libefs_init.c

lib_efs_c_includes := \
	external/openssl/include \
	vendor/intel/efs-tools/include

lib_efs_shared_libraries := \
	libcrypto \
	libkeyutil \
	libcutils \
	liblog

efs_tools_src_files:= \
	src/test.c

efs_tools_shared_libraries := \
	libefs

# keyutil shared library
include $(CLEAR_VARS)
LOCAL_SRC_FILES += $(lib_keyutil_src)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libkeyutil
include $(BUILD_SHARED_LIBRARY)

# libefs shared library
include $(CLEAR_VARS)
LOCAL_SRC_FILES += $(lib_efs_src_files)
LOCAL_C_INCLUDES += $(lib_efs_c_includes)
LOCAL_CFLAGS += -Wall
LOCAL_SHARED_LIBRARIES += $(lib_efs_shared_libraries)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libefs
include $(BUILD_SHARED_LIBRARY)

# libefs_init - small static library to be built with init
include $(CLEAR_VARS)
LOCAL_SRC_FILES += $(lib_efs_init)
LOCAL_CFLAGS += -Wall
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libefs_init
include $(BUILD_STATIC_LIBRARY)

# libefs testing tool
include $(CLEAR_VARS)
LOCAL_MODULE:= efs-tools
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += $(lib_efs_c_includes)
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES := $(efs_tools_src_files)
LOCAL_SHARED_LIBRARIES := $(efs_tools_shared_libraries)
include $(BUILD_EXECUTABLE)
