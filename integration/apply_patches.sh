#!/bin/bash

basepath="external/efs-tools/integration/user"
version=$(repo info Manifest | sed -n 2p | cut -f4 -d ' ')

if [ ! -d "$basepath/$version" ]; then
    echo "Unsupported version"
    exit 1
fi

# force a clean tree before applying patches
repo forall -c git reset --hard HEAD
repo forall -c git clean -df
# Vold integration
cp "$basepath/$version/vold/EncryptedFileStorageCmd.cpp" system/vold/
cp "$basepath/$version/vold/EncryptedFileStorageCmd.h" system/vold/
git apply "$basepath/$version/vold/Vold-integration.patch" --directory=system/vold/
# Core integration
git apply "$basepath/$version/core/Core-integration.patch" --directory=system/core/
# Framework integration
git apply "$basepath/$version/frameworks-base/MountManagerService-integration.patch" --directory=frameworks/base/
# Settings integration
git apply "$basepath/$version/settings/Settings-integration.patch" --directory=packages/apps/Settings/
# SeLinux policy integration
git apply "$basepath/$version/sepolicy/EFS-SeLinux-Policy.patch" --directory=external/sepolicy/
