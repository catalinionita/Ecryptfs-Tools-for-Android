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
if [ $version != "android-5.0.0_r7" ]; then
    git apply "$basepath/$version/core/Core-integration.patch" --directory=system/core/
fi 
# Framework integration
case $version in
    "android-5.0.0_r7" )
        git apply "$basepath/$version/frameworks-base/MountManagerService-integration.patch" --directory=frameworks/base/ ;;
    "android-4.4_r1.2" )
        git apply "$basepath/$version/frameworks-base/MountManagerService-integration.patch" --directory=frameworks/base/ ;;
    "android-4.4.4_r2" )
        git apply "$basepath/$version/frameworks-base/frameworks-base-integration.patch" --directory=frameworks/base/ ;;
esac
# Settings integration
if [ $version != "android-5.0.0_r7" ]; then
    git apply "$basepath/$version/settings/Settings-integration.patch" --directory=packages/apps/Settings/
fi
# SeLinux policy integration
git apply "$basepath/$version/sepolicy/EFS-SeLinux-Policy.patch" --directory=external/sepolicy/
# EFSTest app
if [ $version == "android-5.0.0_r7" ]; then
    git apply "$basepath/$version/build/build.patch" --directory=build/
fi
