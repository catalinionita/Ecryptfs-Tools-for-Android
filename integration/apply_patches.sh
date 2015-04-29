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

case $version in
    "android-5.1.0_r5")
        git apply "$basepath/$version/core/core.patch" --directory=system/core/
        git apply "$basepath/$version/frameworks-base/frameworks-base.patch" --directory=frameworks/base/
        cp "$basepath/$version/frameworks-base/IEFSService.java" frameworks/base/core/java/android/os/storage/
        cp "$basepath/$version/frameworks-base/EFSService.java" frameworks/base/services/core/java/com/android/server/
        ;;
    "android-5.0.2_r1")
        git apply "$basepath/$version/core/core.patch" --directory=system/core/
        git apply "$basepath/$version/frameworks-base/frameworks-base.patch" --directory=frameworks/base/
        cp "$basepath/$version/frameworks-base/IEFSService.java" frameworks/base/core/java/android/os/storage/
        cp "$basepath/$version/frameworks-base/EFSService.java" frameworks/base/services/core/java/com/android/server/
        git apply "$basepath/$version/sepolicy/sepolicy.patch" --directory=external/sepolicy
        cp "$basepath/$version/sepolicy/efs.te" external/sepolicy/
        git apply "$basepath/$version/build/build.patch" --directory=build/
        git apply "$basepath/$version/settings/settings.patch" --directory=packages/apps/Settings/
        ;;
    "android-4.4.4_r2")
        cp "$basepath/$version/vold/EncryptedFileStorageCmd.cpp" system/vold/
        cp "$basepath/$version/vold/EncryptedFileStorageCmd.h" system/vold/
        git apply "$basepath/$version/core/Core-integration.patch" --directory=system/core/
        git apply "$basepath/$version/frameworks-base/frameworks-base-integration.patch" --directory=frameworks/base/
        git apply "$basepath/$version/settings/Settings-integration.patch" --directory=packages/apps/Settings/
        git apply "$basepath/$version/sepolicy/EFS-SeLinux-Policy.patch" --directory=external/sepolicy/
        ;;
    "android-4.4_r1.2")
        cp "$basepath/$version/vold/EncryptedFileStorageCmd.cpp" system/vold/
        cp "$basepath/$version/vold/EncryptedFileStorageCmd.h" system/vold/
	git apply "$basepath/$version/frameworks-base/MountManagerService-integration.patch" --directory=frameworks/base/
        git apply "$basepath/$version/core/Core-integration.patch" --directory=system/core/
        git apply "$basepath/$version/settings/Settings-integration.patch" --directory=packages/apps/Settings/
        git apply "$basepath/$version/sepolicy/EFS-SeLinux-Policy.patch" --directory=external/sepolicy/
        ;;
esac
