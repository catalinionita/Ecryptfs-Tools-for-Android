#!/bin/bash

#vold integration
cp external/efs-tools/integration/vold/EncryptedFileStorageCmd.cpp system/vold/
cp external/efs-tools/integration/vold/EncryptedFileStorageCmd.h system/vold/
git apply external/efs-tools/integration/vold/Vold-integration.patch --directory=system/vold/
#core integration
git apply external/efs-tools/integration/core/Core-integration.patch --directory=system/core/
#framework integration
git apply external/efs-tools/integration/frameworks-base/MountManagerService-integration.patch --directory=frameworks/base/
#Settings integration
git apply external/efs-tools/integration/settings/Settings-integration.patch --directory=packages/apps/Settings/
